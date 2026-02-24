#define _USE_MATH_DEFINES
#include <cmath>
#include "Dialect/QNet/Passes.h"
#include "Dialect/QNet/QNet.h"
#include "llvm/Support/Debug.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

#define DEBUG_TYPE "qnet-peephole-optimizations-pass"

using namespace mlir;
using namespace qoala::helpers;
using namespace qoala::dialects::qnet;

namespace qoala::analysis {
#define GEN_PASS_DEF_QNETPEEPHOLEOPTIMIZATIONS
#include "Dialect/QNet/Passes.h.inc"

    struct CancelHermitianPairPattern : OpInterfaceRewritePattern<HermitianOpIface> {
        using OpInterfaceRewritePattern::OpInterfaceRewritePattern;

        LogicalResult matchAndRewrite(HermitianOpIface opIface, PatternRewriter &rewriter) const override {
            Operation *op = opIface.getOperation();
            LLVM_DEBUG(llvm::dbgs() << "[HermitianCancel] Checking op: " << op->getName() << " @" << op->getLoc()
                                    << "\n");

            // Must have operands and results
            if (op->getNumOperands() == 0 || op->getNumResults() == 0) {
                LLVM_DEBUG(llvm::dbgs() << "[HermitianCancel]   -> Skip: no operands/results\n");
                return failure();
            }

            // Candidate previous op (def of first operand)
            // NOTE: Currently we use the first operand to identify the candidate producer.
            // This is safe for existing single and two-qubit Hermitian gates, as later
            // checks enforce operand/result alignment. For future ops with more complex
            // wiring, consider verifying all operands share the same defining op.
            Operation *prevOp = op->getOperand(0).getDefiningOp();
            if (!prevOp) {
                LLVM_DEBUG(llvm::dbgs() << "[HermitianCancel]   -> Skip: first operand has no defining op\n");
                return failure();
            }

            // prevOp must also implement the same interface
            if (!isa<HermitianOpIface>(prevOp)) {
                LLVM_DEBUG(llvm::dbgs() << "[HermitianCancel]   -> Skip: previous op not Hermitian\n");
                return failure();
            }

            // Same op class/kind
            if (prevOp->getName() != op->getName()) {
                LLVM_DEBUG(llvm::dbgs() << "[HermitianCancel]   -> Skip: different op types (" << prevOp->getName()
                                        << " vs " << op->getName() << ")\n");
                return failure();
            }

            // Quick structural sanity check: make sure both ops have the same
            // number of wires (results/operands). The detailed per-wire alignment
            // and single-use checks happen right after.
            if (prevOp->getNumResults() != op->getNumOperands() || prevOp->getNumOperands() != op->getNumResults()) {
                LLVM_DEBUG(llvm::dbgs() << "[HermitianCancel]   -> Skip: mismatched arity\n");
                return failure();
            }

            for (auto [opd, resPrev, resCurr, inPrev] :
                 llvm::zip(op->getOperands(), prevOp->getResults(), op->getResults(), prevOp->getOperands())) {
                // Strict adjacency and single-use of prev results
                if (opd != resPrev) {
                    LLVM_DEBUG(llvm::dbgs() << "[HermitianCancel]   -> Skip: operands/results not aligned\n");
                    return failure();
                }
                if (!resPrev.hasOneUse()) {
                    LLVM_DEBUG(llvm::dbgs() << "[HermitianCancel]   -> Skip: result has multiple uses\n");
                    return failure();
                }

                // Type sanity: each result of 'op' must match the corresponding input of 'prevOp'
                if (resCurr.getType() != inPrev.getType()) {
                    LLVM_DEBUG(llvm::dbgs() << "[HermitianCancel]   -> Skip: result/input type mismatch\n");
                    return failure();
                }
            }

            LLVM_DEBUG(llvm::dbgs() << "[HermitianCancel]   Matched pair: " << prevOp->getName() << " / "
                                    << op->getName() << " -> performing cancellation\n");

            // Forward past BOTH ops: replace 'op' results with the ORIGINAL inputs of 'prevOp'.
            // This ensures 'prevOp' becomes dead (no uses) and can be erased safely.
            rewriter.replaceOp(op, prevOp->getOperands());

            LLVM_DEBUG({
                llvm::dbgs() << "[HermitianCancel]      forwarded results of " << op->getName()
                             << " to inputs of previous " << prevOp->getName() << " [";
                llvm::interleaveComma(prevOp->getOperands(), llvm::dbgs());
                llvm::dbgs() << "]\n";
            });

            // Now safe to erase prevOp (its results were only used by 'op').
            rewriter.eraseOp(prevOp);

            LLVM_DEBUG(llvm::dbgs() << "[HermitianCancel]   Cancellation complete\n");
            return success();
        }
    };

    static Value materializePiF32(PatternRewriter &rewriter, Location loc) {
        constexpr auto pi = static_cast<float>(M_PI);
        auto piAttr = FloatAttr::get(rewriter.getF32Type(), pi);
        return rewriter.create<arith::ConstantOp>(loc, rewriter.getF32Type(), piAttr).getResult();
    }

    // PauliOpT must implement QubitOpIface.
    // RotOpT is assumed to take operands: (qubit, angle)
    template<typename PauliOpT, typename RotOpT>
    struct PauliToRotationPattern final : OpRewritePattern<PauliOpT> {
        using OpRewritePattern<PauliOpT>::OpRewritePattern;

        LogicalResult matchAndRewrite(PauliOpT pauli, PatternRewriter &rewriter) const override {
            Operation *op = pauli.getOperation();

            auto qubitIface = dyn_cast<QubitOpIface>(op);
            if (!qubitIface) {
                return failure();
            }

            // Only handle single-qubit Paulis
            // NOTE: this should be doable on CZ but we do not implement CRotZ
            if (qubitIface.isTwoQubitOp()) {
                return failure();
            }

            const OperandRange qops = qubitIface.getQubitOperands();
            if (qops.size() != 1) {
                LLVM_DEBUG(llvm::dbgs() << "[PauliToRot] Skip: expected 1 qubit operand, got " << qops.size() << " for "
                                        << op->getName() << "\n");
                return failure();
            }

            const Value qubit = qops.front();
            const Value piVal = materializePiF32(rewriter, pauli.getLoc());

            // Rot*Op expects (qubit, angle) as operands => 2 operands total.
            SmallVector<Value, 2> rotOperands{qubit, piVal};

            auto rot = rewriter.create<RotOpT>(pauli.getLoc(), op->getResultTypes(), rotOperands);

            rewriter.replaceOp(op, rot->getResults());
            return success();
        }
    };

    struct FoldRotationPairPattern : OpInterfaceRewritePattern<RotationOpIface> {
        using OpInterfaceRewritePattern::OpInterfaceRewritePattern;

        LogicalResult matchAndRewrite(RotationOpIface rot, PatternRewriter &rewriter) const override {
            Operation *op = rot.getOperation();
            LLVM_DEBUG(llvm::dbgs() << "[RotFold] Checking op: " << op->getName() << " @" << op->getLoc() << "\n");

            // Must have a target operand that comes from a previous rotation
            const Value target = rot.getTarget();
            Operation *prevOp = target ? target.getDefiningOp() : nullptr;
            if (!prevOp) {
                LLVM_DEBUG(llvm::dbgs() << "[RotFold]   -> Skip: target has no defining op\n");
                return failure();
            }

            // Previous must also be a rotation
            auto prevRot = dyn_cast<RotationOpIface>(prevOp);
            if (!prevRot) {
                LLVM_DEBUG(llvm::dbgs() << "[RotFold]   -> Skip: previous op not a rotation\n");
                return failure();
            }

            // Same axis
            if (prevRot.getAxis() != rot.getAxis()) {
                LLVM_DEBUG(llvm::dbgs() << "[RotFold]   -> Skip: axis mismatch\n");
                return failure();
            }

            // Controls “match” in the controlled case:
            // We accept either exact SSA equality (trivial) or the common pass-through
            // case where `rot` takes as controls the *results* of `prevOp`.
            const ValueRange prevCtrls = prevRot.getControls();
            const ValueRange ctrls = rot.getControls();
            if (prevCtrls.size() != ctrls.size()) {
                LLVM_DEBUG(llvm::dbgs() << "[RotFold]   -> Skip: control arity mismatch\n");
                return failure();
            }

            const uint32_t numCtrls = ctrls.size();
            const uint32_t tgtIdx = rot.getTargetOperandIndex();

            if (prevOp->getNumResults() != (numCtrls + 1)) {
                LLVM_DEBUG(llvm::dbgs() << "[RotFold]   -> Skip: prev result arity " << prevOp->getNumResults()
                                        << " != numCtrls+1 (" << (numCtrls + 1) << ")\n");
                return failure();
            }

            // Ensure the “pass-through” wiring: each control operand of `op`
            // is exactly the corresponding result of `prevOp`.
            for (uint32_t i = 0; i < numCtrls; ++i) {
                if (op->getOperand(i) != prevOp->getResult(i)) {
                    LLVM_DEBUG(llvm::dbgs()
                               << "[RotFold]   -> Skip: control #" << i << " not fed by prev result #" << i << "\n");
                    return failure();
                }
            }

            // The target we already know comes from prevOp (by how we found prevOp),
            // but we still assert it matches the expected result slot.
            if (op->getOperand(tgtIdx) != prevOp->getResult(numCtrls)) {
                LLVM_DEBUG(llvm::dbgs() << "[RotFold]   -> Skip: target operand #" << tgtIdx
                                        << " not fed by prev target result #" << numCtrls << "\n");
                return failure();
            }

            // Every result of prevOp must be uniquely used by `op`, so we can erase it.
            if (!llvm::all_of(prevOp->getResults(), [&](const OpResult r) { return r.hasOneUse(); })) {
                LLVM_DEBUG(llvm::dbgs() << "[RotFold]   -> Skip: prev result has extra uses\n");
                return failure();
            }

            // Angles must be constants we can fold
            const auto a1Attr = dyn_cast_or_null<FloatAttr>(prevRot.getAngleAttr());
            const auto a2Attr = dyn_cast_or_null<FloatAttr>(rot.getAngleAttr());
            if (!a1Attr || !a2Attr) {
                LLVM_DEBUG(llvm::dbgs() << "[RotFold]   -> Skip: non-constant angles (prev="
                                        << (a1Attr ? "const" : "non-const")
                                        << ", this=" << (a2Attr ? "const" : "non-const") << ")\n");
                return failure();
            }

            APFloat sum = a1Attr.getValue();
            sum.add(a2Attr.getValue(), APFloat::rmNearestTiesToEven);

            // Mutations:
            //  - set new angle on the second op
            const auto newAngleAttr = FloatAttr::get(a2Attr.getType(), sum);
            rot.setAngleAttr(newAngleAttr);

            //  - rewire target of `op` to previous op's *input* target
            //  (i.e., bypass prevOp)
            const Value prevInputTarget = prevRot.getTarget();
            op->setOperand(tgtIdx, prevInputTarget);

            //  - rewire all control operands of `op` to previous op's *input* controls
            //  The iface default says controls are the leading operands.
            for (uint32_t i = 0; i < numCtrls; ++i) {
                op->setOperand(i, prevOp->getOperand(i));
            }

            // Safe to erase previous rotation
            rewriter.eraseOp(prevOp);
            return success();
        }
    };

    static double normalizeToMinusPiPi(double x) {
        // Canonical range: (-pi, pi]
        constexpr double twoPi = 2.0 * M_PI;

        // remainder in (-2pi, 2pi) similar to fmod
        x = std::remainder(x, twoPi); // gives result in [-pi, pi]

        // We want (-pi, pi], so map -pi to +pi
        if (x <= -M_PI) {
            x += twoPi;
        }
        // Now x in (-pi, pi]
        return x;
    }

    struct NormalizeRotationAnglePattern final : OpInterfaceRewritePattern<RotationOpIface> {
        using OpInterfaceRewritePattern::OpInterfaceRewritePattern;

        LogicalResult matchAndRewrite(RotationOpIface rot, PatternRewriter &rewriter) const override {

            const auto aAttr = dyn_cast_or_null<FloatAttr>(rot.getAngleAttr());
            if (!aAttr) {
                return failure();
            }

            // Convert to double, normalize
            const double a = aAttr.getValue().convertToDouble();
            if (!std::isfinite(a)) {
                return failure();
            }

            const double norm = normalizeToMinusPiPi(a);

            // If already canonical (exactly equal), do nothing.
            // Note: for floats, equality is OK here because we are rewriting constants
            // and want to avoid churn. If you prefer, compare within a tiny tolerance.
            if (norm == a) {
                return failure();
            }

            // Rebuild constant with same float type as original attribute
            const Type ty = aAttr.getType();
            const FloatAttr newAttr = FloatAttr::get(ty, norm);

            rot.setAngleAttr(newAttr);
            return success();
        }
    };

    static bool isMultipleOfTwoPi(const FloatAttr angleAttr, const double epsilon) {
        if (!angleAttr) {
            return false;
        }
        if (!(epsilon >= 0.0)) {
            return false; // NaN-safe, and negative eps disabled
        }

        constexpr double twoPi = 2.0 * M_PI;

        // Convert APFloat -> double
        const double a = angleAttr.getValue().convertToDouble();

        // Handle non-finite values defensively
        if (!std::isfinite(a)) {
            return false;
        }

        // Find nearest integer multiple k of 2pi
        const double k = std::nearbyint(a / twoPi);
        const double diff = std::fabs(a - k * twoPi);

        return diff <= epsilon;
    }

    struct EliminateFullTurnRotationPattern final : OpInterfaceRewritePattern<RotationOpIface> {
        explicit EliminateFullTurnRotationPattern(MLIRContext *ctx, const double epsilon,
                                                  const PatternBenefit benefit = 1):
            OpInterfaceRewritePattern(ctx, benefit), epsilon(epsilon) { }

        LogicalResult matchAndRewrite(RotationOpIface rot, PatternRewriter &rewriter) const override {
            Operation *op = rot.getOperation();

            const ValueRange ctrls = rot.getControls();
            if (!ctrls.empty()) {
                LLVM_DEBUG(llvm::dbgs() << "[Rot2Pi]   -> Skip: controlled rotation (" << ctrls.size()
                                        << " controls)\n");
                return failure();
            }

            const auto aAttr = dyn_cast_or_null<FloatAttr>(rot.getAngleAttr());
            if (!aAttr) {
                return failure();
            }

            if (!isMultipleOfTwoPi(aAttr, epsilon)) {
                return failure();
            }

            // Uncontrolled => passthrough target only
            const Value tgt = rot.getTarget();
            if (op->getNumResults() != 1) {
                return failure();
            }

            rewriter.replaceOp(op, tgt);
            return success();
        }

    private:
        double epsilon;
    };

    // ============================================================
    // Int-rotation helpers
    // ============================================================

    static std::optional<int32_t> getConstI32(Value v) {
        auto cst = v.getDefiningOp<arith::ConstantOp>();
        if (!cst) {
            return std::nullopt;
        }
        const auto ia = dyn_cast<IntegerAttr>(cst.getValue());
        if (!ia) {
            return std::nullopt;
        }
        return static_cast<int32_t>(ia.getInt());
    }

    static Value makeI32Const(PatternRewriter &rewriter, Location loc, int32_t val) {
        return rewriter.create<arith::ConstantOp>(loc, rewriter.getI32IntegerAttr(val)).getResult();
    }

    // Combine two int rotation angles (n1/2^d1 + n2/2^d2) into a single (n_new, d_new).
    // Uses a constant fast-path when all four values are compile-time constants (with
    // overflow detection), and falls back to emitting arith ops for dynamic values.
    // Returns std::nullopt only for the constant path when overflow is detected.
    // The caller must have set the rewriter insertion point before calling this.
    static std::optional<std::pair<Value, Value>>
    combineIntRotAngles(PatternRewriter &rewriter, Location loc, Value n1Val, Value d1Val, Value n2Val, Value d2Val) {
        const auto c_n1 = getConstI32(n1Val);
        const auto c_d1 = getConstI32(d1Val);
        const auto c_n2 = getConstI32(n2Val);
        const auto c_d2 = getConstI32(d2Val);

        if (c_n1 && c_d1 && c_n2 && c_d2) {
            // Constant fast-path
            if (*c_d1 < 0 || *c_d2 < 0) {
                return std::nullopt;
            }
            const int32_t d_new = std::max(*c_d1, *c_d2);
            const int32_t diff1 = d_new - *c_d1;
            const int32_t diff2 = d_new - *c_d2;
            if (diff1 >= 31 || diff2 >= 31) {
                return std::nullopt;
            }
            const int64_t n_sum = (static_cast<int64_t>(*c_n1) << diff1) + (static_cast<int64_t>(*c_n2) << diff2);
            const auto n_new = static_cast<int32_t>(n_sum);
            if (static_cast<int64_t>(n_new) != n_sum) {
                return std::nullopt; // overflows i32
            }
            return std::make_pair(makeI32Const(rewriter, loc, n_new), makeI32Const(rewriter, loc, d_new));
        }

        // Dynamic path: emit arith ops.
        // d_new = max(d1, d2)
        Value dNew = rewriter.create<arith::MaxSIOp>(loc, d1Val, d2Val);
        // delta_i = d_new - d_i  (0 for the larger denominator)
        Value delta1 = rewriter.create<arith::SubIOp>(loc, dNew, d1Val);
        Value delta2 = rewriter.create<arith::SubIOp>(loc, dNew, d2Val);
        // Scale each numerator to the common denominator
        Value n1Shifted = rewriter.create<arith::ShLIOp>(loc, n1Val, delta1);
        Value n2Shifted = rewriter.create<arith::ShLIOp>(loc, n2Val, delta2);
        // n_new = n1_scaled + n2_scaled
        Value nNew = rewriter.create<arith::AddIOp>(loc, n1Shifted, n2Shifted);
        return std::make_pair(nNew, dNew);
    }

    // ============================================================
    // Fold adjacent mixed float/int rotations on the same axis.
    // Both cases produce a FloatRotOpT with combined angle = float_angle + n·π/2^d.
    // Only fires when the float angle and both int params are compile-time constants.
    //
    //  FloatRot(qin, a)  → IntRot(_, n, d)   (float first, int second)
    //  IntRot(qin, n, d) → FloatRot(_, a)    (int first, float second)
    // ============================================================
    template<typename FloatRotOpT, typename IntRotOpT>
    struct FoldFloatThenIntRotationPattern final : OpRewritePattern<IntRotOpT> {
        using OpRewritePattern<IntRotOpT>::OpRewritePattern;

        LogicalResult matchAndRewrite(IntRotOpT rot, PatternRewriter &rewriter) const override {
            Operation *prevOp = rot.getQin().getDefiningOp();
            if (!prevOp) {
                return failure();
            }

            auto prevFloat = dyn_cast<FloatRotOpT>(prevOp);
            if (!prevFloat) {
                return failure();
            }

            if (!prevOp->getResult(0).hasOneUse()) {
                return failure();
            }

            // Float angle must be a compile-time constant
            auto angleCst = prevFloat.getAngle().template getDefiningOp<arith::ConstantOp>();
            if (!angleCst) {
                return failure();
            }
            const auto aAttr = dyn_cast<FloatAttr>(angleCst.getValue());
            if (!aAttr) {
                return failure();
            }

            // Int params must be compile-time constants
            const auto nOpt = getConstI32(rot.getNVal());
            const auto dOpt = getConstI32(rot.getExpVal());
            if (!nOpt || !dOpt || *dOpt < 0) {
                return failure();
            }

            // combined = float_angle + n·π / 2^d
            const double intAngle = static_cast<double>(*nOpt) * M_PI / std::pow(2.0, static_cast<double>(*dOpt));
            const double combined = aAttr.getValue().convertToDouble() + intAngle;

            rewriter.setInsertionPoint(rot.getOperation());
            Value newAngle =
                    rewriter.create<arith::ConstantOp>(rot.getLoc(), FloatAttr::get(rewriter.getF32Type(), combined))
                            .getResult();

            SmallVector<Value, 2> operands{prevFloat.getQin(), newAngle};
            auto newRot = rewriter.create<FloatRotOpT>(rot.getLoc(), rot.getOperation()->getResultTypes(), operands);

            rewriter.replaceOp(rot, newRot->getResults());
            rewriter.eraseOp(prevOp);
            return success();
        }
    };

    template<typename FloatRotOpT, typename IntRotOpT>
    struct FoldIntThenFloatRotationPattern final : OpRewritePattern<FloatRotOpT> {
        using OpRewritePattern<FloatRotOpT>::OpRewritePattern;

        LogicalResult matchAndRewrite(FloatRotOpT rot, PatternRewriter &rewriter) const override {
            Operation *prevOp = rot.getQin().getDefiningOp();
            if (!prevOp) {
                return failure();
            }

            auto prevInt = dyn_cast<IntRotOpT>(prevOp);
            if (!prevInt) {
                return failure();
            }

            if (!prevOp->getResult(0).hasOneUse()) {
                return failure();
            }

            // Float angle must be a compile-time constant
            auto angleCst = rot.getAngle().template getDefiningOp<arith::ConstantOp>();
            if (!angleCst) {
                return failure();
            }
            const auto aAttr = dyn_cast<FloatAttr>(angleCst.getValue());
            if (!aAttr) {
                return failure();
            }

            // Int params must be compile-time constants
            const auto nOpt = getConstI32(prevInt.getNVal());
            const auto dOpt = getConstI32(prevInt.getExpVal());
            if (!nOpt || !dOpt || *dOpt < 0) {
                return failure();
            }

            // combined = n·π / 2^d + float_angle
            const double intAngle = static_cast<double>(*nOpt) * M_PI / std::pow(2.0, static_cast<double>(*dOpt));
            const double combined = intAngle + aAttr.getValue().convertToDouble();

            rewriter.setInsertionPoint(rot.getOperation());
            Value newAngle =
                    rewriter.create<arith::ConstantOp>(rot.getLoc(), FloatAttr::get(rewriter.getF32Type(), combined))
                            .getResult();

            SmallVector<Value, 2> operands{prevInt.getQin(), newAngle};
            auto newRot = rewriter.create<FloatRotOpT>(rot.getLoc(), rot.getOperation()->getResultTypes(), operands);

            rewriter.replaceOp(rot, newRot->getResults());
            rewriter.eraseOp(prevOp);
            return success();
        }
    };

    // ============================================================
    // Fold adjacent same-axis int rotations:
    //   rot(qin, n1, d1) → rot(_, n2, d2)  ⟹  rot(qin, n_new, d_new)
    // where d_new = max(d1, d2), n_new = n1·2^(d_new−d1) + n2·2^(d_new−d2).
    // Only fires when all four operands are compile-time constants.
    // ============================================================
    template<typename IntRotOpT>
    struct FoldIntRotationPairPattern final : OpRewritePattern<IntRotOpT> {
        using OpRewritePattern<IntRotOpT>::OpRewritePattern;

        LogicalResult matchAndRewrite(IntRotOpT rot, PatternRewriter &rewriter) const override {
            Operation *prevOp = rot.getQin().getDefiningOp();
            if (!prevOp) {
                return failure();
            }

            auto prevRot = dyn_cast<IntRotOpT>(prevOp);
            if (!prevRot) {
                return failure();
            }

            // prevOp's single qubit result must feed only this op
            if (!prevOp->getResult(0).hasOneUse()) {
                return failure();
            }

            rewriter.setInsertionPoint(rot.getOperation());
            const auto combined = combineIntRotAngles(rewriter, rot.getLoc(), prevRot.getNVal(), prevRot.getExpVal(),
                                                      rot.getNVal(), rot.getExpVal());
            if (!combined) {
                return failure(); // constant-path overflow
            }

            // Mutate rot in-place: operands are 0=qin, 1=nVal, 2=expVal
            rot.getOperation()->setOperand(0, prevRot.getQin());
            rot.getOperation()->setOperand(1, combined->first);
            rot.getOperation()->setOperand(2, combined->second);

            rewriter.eraseOp(prevOp);
            return success();
        }
    };

    struct FoldCrotXIntRotationPairPattern final : OpRewritePattern<CrotXIntOp> {
        using OpRewritePattern::OpRewritePattern;

        LogicalResult matchAndRewrite(CrotXIntOp rot, PatternRewriter &rewriter) const override {
            // Target qubit is qin1 (operand 1)
            Operation *prevOp = rot.getQin1().getDefiningOp();
            if (!prevOp) {
                return failure();
            }

            auto prevRot = dyn_cast<CrotXIntOp>(prevOp);
            if (!prevRot) {
                return failure();
            }

            // Both results of prevOp must be consumed only by this op
            if (!llvm::all_of(prevOp->getResults(), [](OpResult r) { return r.hasOneUse(); })) {
                return failure();
            }

            // Control wiring: rot.qin0 must come from prevOp.qout0
            if (rot.getQin0() != prevOp->getResult(0)) {
                return failure();
            }

            rewriter.setInsertionPoint(rot.getOperation());
            const auto combined = combineIntRotAngles(rewriter, rot.getLoc(), prevRot.getNVal(), prevRot.getExpVal(),
                                                      rot.getNVal(), rot.getExpVal());
            if (!combined) {
                return failure(); // constant-path overflow
            }

            // Mutate in-place: operands are 0=qin0, 1=qin1, 2=nVal, 3=expVal
            rot.getOperation()->setOperand(0, prevRot.getQin0());
            rot.getOperation()->setOperand(1, prevRot.getQin1());
            rot.getOperation()->setOperand(2, combined->first);
            rot.getOperation()->setOperand(3, combined->second);

            rewriter.eraseOp(prevOp);
            return success();
        }
    };

    // ============================================================
    // Normalize int rotation: reduce n modulo 2^(d+1) into (−2^d, 2^d].
    // Only fires when both n and d are compile-time constants and d ∈ [0, 30].
    // For d ≥ 31, any i32 value of n already satisfies |n| < 2^d (canonical).
    // ============================================================
    template<typename IntRotOpT>
    struct NormalizeIntRotationPattern final : OpRewritePattern<IntRotOpT> {
        using OpRewritePattern<IntRotOpT>::OpRewritePattern;

        LogicalResult matchAndRewrite(IntRotOpT rot, PatternRewriter &rewriter) const override {
            const auto nOpt = getConstI32(rot.getNVal());
            const auto dOpt = getConstI32(rot.getExpVal());
            if (!nOpt || !dOpt) {
                return failure();
            }

            const int32_t n = *nOpt;
            const int32_t d = *dOpt;
            if (d < 0 || d >= 31) {
                return failure();
            }

            const int32_t period = 1 << (d + 1); // 2^(d+1)
            const int32_t half = 1 << d; // 2^d

            // Reduce n into [0, period), then shift to (−half, half]
            const int32_t n_mod = ((n % period) + period) % period;
            const int32_t n_norm = (n_mod > half) ? (n_mod - period) : n_mod;

            if (n_norm == n) {
                return failure(); // already canonical
            }

            rewriter.setInsertionPoint(rot.getOperation());
            // Operands: 0=qin, 1=nVal, 2=expVal
            rot.getOperation()->setOperand(1, makeI32Const(rewriter, rot.getLoc(), n_norm));
            return success();
        }
    };

    struct NormalizeCrotXIntRotationPattern final : OpRewritePattern<CrotXIntOp> {
        using OpRewritePattern::OpRewritePattern;

        LogicalResult matchAndRewrite(CrotXIntOp rot, PatternRewriter &rewriter) const override {
            const auto nOpt = getConstI32(rot.getNVal());
            const auto dOpt = getConstI32(rot.getExpVal());
            if (!nOpt || !dOpt) {
                return failure();
            }

            const int32_t n = *nOpt;
            const int32_t d = *dOpt;
            if (d < 0 || d >= 31) {
                return failure();
            }

            const int32_t period = 1 << (d + 1);
            const int32_t half = 1 << d;

            const int32_t n_mod = ((n % period) + period) % period;
            const int32_t n_norm = (n_mod > half) ? (n_mod - period) : n_mod;

            if (n_norm == n) {
                return failure();
            }

            rewriter.setInsertionPoint(rot.getOperation());
            // Operands: 0=qin0, 1=qin1, 2=nVal, 3=expVal
            rot.getOperation()->setOperand(2, makeI32Const(rewriter, rot.getLoc(), n_norm));
            return success();
        }
    };

    // ============================================================
    // Eliminate zero int rotation: n == 0  →  angle == 0  →  identity.
    // Gated on twoPiEpsilon (consistent with float full-turn elimination).
    // ============================================================
    template<typename IntRotOpT>
    struct EliminateZeroIntRotationPattern final : OpRewritePattern<IntRotOpT> {
        using OpRewritePattern<IntRotOpT>::OpRewritePattern;

        LogicalResult matchAndRewrite(IntRotOpT rot, PatternRewriter &rewriter) const override {
            const auto nOpt = getConstI32(rot.getNVal());
            if (!nOpt || *nOpt != 0) {
                return failure();
            }

            rewriter.replaceOp(rot, rot.getQin());
            return success();
        }
    };

    struct EliminateZeroCrotXIntRotationPattern final : OpRewritePattern<CrotXIntOp> {
        using OpRewritePattern::OpRewritePattern;

        LogicalResult matchAndRewrite(CrotXIntOp rot, PatternRewriter &rewriter) const override {
            const auto nOpt = getConstI32(rot.getNVal());
            if (!nOpt || *nOpt != 0) {
                return failure();
            }

            rewriter.replaceOp(rot, {rot.getQin0(), rot.getQin1()});
            return success();
        }
    };

    class QNetPeepholeOptimizationsPass : public impl::QNetPeepholeOptimizationsBase<QNetPeepholeOptimizationsPass> {
        using QNetPeepholeOptimizationsBase::QNetPeepholeOptimizationsBase;

        void runOnOperation() override;
    };

    void QNetPeepholeOptimizationsPass::runOnOperation() {
        LLVM_DEBUG(llvm::dbgs() << "[QNet][Peephole Optimizations] starts, hermitianCancel=" << this->hermitianCancel
                                << ", rotationFold=" << this->rotationFolding << ", pauliToRotations="
                                << this->pauliGatesToRotations << ", twoPiEpsilon=" << this->twoPiEpsilon
                                << "normalizeAngles=" << this->normalizeAngles << "\n");

        RewritePatternSet patterns(&getContext());
        if (this->hermitianCancel) {
            patterns.add<CancelHermitianPairPattern>(&getContext());
        }
        if (this->pauliGatesToRotations) {
            patterns.add<PauliToRotationPattern<XOp, RotXOp>, PauliToRotationPattern<YOp, RotYOp>,
                         PauliToRotationPattern<ZOp, RotZOp>>(&getContext());
        }
        if (this->rotationFolding) {
            patterns.add<FoldRotationPairPattern>(&getContext());
            patterns.add<FoldIntRotationPairPattern<RotXIntOp>, FoldIntRotationPairPattern<RotYIntOp>,
                         FoldIntRotationPairPattern<RotZIntOp>, FoldCrotXIntRotationPairPattern>(&getContext());
            patterns.add<FoldFloatThenIntRotationPattern<RotXOp, RotXIntOp>,
                         FoldFloatThenIntRotationPattern<RotYOp, RotYIntOp>,
                         FoldFloatThenIntRotationPattern<RotZOp, RotZIntOp>,
                         FoldIntThenFloatRotationPattern<RotXOp, RotXIntOp>,
                         FoldIntThenFloatRotationPattern<RotYOp, RotYIntOp>,
                         FoldIntThenFloatRotationPattern<RotZOp, RotZIntOp>>(&getContext());
        }
        if (this->normalizeAngles) {
            patterns.add<NormalizeRotationAnglePattern>(&getContext());
            patterns.add<NormalizeIntRotationPattern<RotXIntOp>, NormalizeIntRotationPattern<RotYIntOp>,
                         NormalizeIntRotationPattern<RotZIntOp>, NormalizeCrotXIntRotationPattern>(&getContext());
        }
        if (this->twoPiEpsilon >= 0.0) {
            patterns.add<EliminateFullTurnRotationPattern>(&getContext(), this->twoPiEpsilon);
            patterns.add<EliminateZeroIntRotationPattern<RotXIntOp>, EliminateZeroIntRotationPattern<RotYIntOp>,
                         EliminateZeroIntRotationPattern<RotZIntOp>, EliminateZeroCrotXIntRotationPattern>(
                    &getContext());
        }

        constexpr GreedyRewriteConfig cfg;
        (void) applyPatternsAndFoldGreedily(getOperation(), std::move(patterns), cfg);

        LLVM_DEBUG(llvm::dbgs() << "[QNet][Peephole] finished\n");
    }

} // namespace qoala::analysis
