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

    static mlir::Value materializePiF32(mlir::PatternRewriter &rewriter, mlir::Location loc) {
        const float pi = static_cast<float>(M_PI);
        auto piAttr = mlir::FloatAttr::get(rewriter.getF32Type(), pi);
        return rewriter.create<mlir::arith::ConstantOp>(loc, rewriter.getF32Type(), piAttr).getResult();
    }

    // PauliOpT must implement QubitOpIface.
    // RotOpT is assumed to take operands: (qubit, angle)
    template<typename PauliOpT, typename RotOpT>
    struct PauliToRotationPattern final : mlir::OpRewritePattern<PauliOpT> {
        using mlir::OpRewritePattern<PauliOpT>::OpRewritePattern;

        mlir::LogicalResult matchAndRewrite(PauliOpT pauli, mlir::PatternRewriter &rewriter) const override {
            mlir::Operation *op = pauli.getOperation();

            auto qubitIface = mlir::dyn_cast<qoala::helpers::QubitOpIface>(op);
            if (!qubitIface) {
                return mlir::failure();
            }

            // Only handle single-qubit Paulis
            // NOTE: this should be doable on CZ but we do not implement CRotZ
            if (qubitIface.isTwoQubitOp()) {
                return mlir::failure();
            }

            mlir::OperandRange qops = qubitIface.getQubitOperands();
            if (qops.size() != 1) {
                LLVM_DEBUG(llvm::dbgs() << "[PauliToRot] Skip: expected 1 qubit operand, got " << qops.size() << " for "
                                        << op->getName() << "\n");
                return mlir::failure();
            }

            mlir::Value qubit = qops.front();
            mlir::Value piVal = materializePiF32(rewriter, pauli.getLoc());

            // Rot*Op expects (qubit, angle) as operands => 2 operands total.
            llvm::SmallVector<mlir::Value, 2> rotOperands{qubit, piVal};

            auto rot = rewriter.create<RotOpT>(pauli.getLoc(), op->getResultTypes(), rotOperands);

            rewriter.replaceOp(op, rot->getResults());
            return mlir::success();
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

    static bool isMultipleOfTwoPi(const mlir::FloatAttr angleAttr, double epsilon) {
        if (!angleAttr) {
            return false;
        }
        if (!(epsilon >= 0.0)) {
            return false; // NaN-safe, and negative eps disables
        }

        const double twoPi = 2.0 * M_PI;

        // Convert APFloat -> double
        const double a = angleAttr.getValue().convertToDouble();

        // Handle non-finite values defensively
        if (!std::isfinite(a)) {
            return false;
        }

        // Find nearest integer multiple k of 2π
        const double k = std::nearbyint(a / twoPi);
        const double diff = std::fabs(a - k * twoPi);

        return diff <= epsilon;
    }

    struct EliminateFullTurnRotationPattern final : mlir::OpInterfaceRewritePattern<RotationOpIface> {
        explicit EliminateFullTurnRotationPattern(mlir::MLIRContext *ctx, double epsilon,
                                                  mlir::PatternBenefit benefit = 1):
            OpInterfaceRewritePattern<RotationOpIface>(ctx, benefit), epsilon(epsilon) { }

        mlir::LogicalResult matchAndRewrite(RotationOpIface rot, mlir::PatternRewriter &rewriter) const override {
            mlir::Operation *op = rot.getOperation();

            const mlir::ValueRange ctrls = rot.getControls();
            if (!ctrls.empty()) {
                LLVM_DEBUG(llvm::dbgs() << "[Rot2Pi]   -> Skip: controlled rotation (" << ctrls.size()
                                        << " controls)\n");
                return mlir::failure();
            }

            const auto aAttr = mlir::dyn_cast_or_null<mlir::FloatAttr>(rot.getAngleAttr());
            if (!aAttr) {
                return mlir::failure();
            }

            if (!isMultipleOfTwoPi(aAttr, epsilon)) {
                return mlir::failure();
            }

            // Uncontrolled => passthrough target only
            mlir::Value tgt = rot.getTarget();
            if (op->getNumResults() != 1) {
                return mlir::failure();
            }

            rewriter.replaceOp(op, tgt);
            return mlir::success();
        }

    private:
        double epsilon;
    };

    class QNetPeepholeOptimizationsPass : public impl::QNetPeepholeOptimizationsBase<QNetPeepholeOptimizationsPass> {
        using QNetPeepholeOptimizationsBase::QNetPeepholeOptimizationsBase;

        void runOnOperation() override;
    };

    void QNetPeepholeOptimizationsPass::runOnOperation() {
        LLVM_DEBUG(llvm::dbgs() << "[QNet][Peephole Optimizations] starts, hermitianCancel=" << this->hermitianCancel
                                << ", rotationFold=" << this->rotationFolding << ", pauliToRotations="
                                << this->pauliGatesToRotations << ", twoPiEpsilon=" << this->twoPiEpsilon << "\n");

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
        }
        if (this->twoPiEpsilon >= 0.0) {
            patterns.add<EliminateFullTurnRotationPattern>(&getContext(), this->twoPiEpsilon);
        }

        GreedyRewriteConfig cfg;
        (void) applyPatternsAndFoldGreedily(getOperation(), std::move(patterns), cfg);

        LLVM_DEBUG(llvm::dbgs() << "[QNet][Peephole] finished\n");
    }

} // namespace qoala::analysis
