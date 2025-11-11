#include "Dialect/QNet/Passes.h"
#include "Dialect/QNet/QNet.h"
#include "llvm/Support/Debug.h"
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
        using Base = OpInterfaceRewritePattern<HermitianOpIface>;
        using Base::Base;

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

    struct FoldRotationPairPattern : OpInterfaceRewritePattern<RotationOpIface> {
        using Base = OpInterfaceRewritePattern<RotationOpIface>;
        using Base::Base;

        LogicalResult matchAndRewrite(RotationOpIface rot, PatternRewriter &rewriter) const override {
            Operation *op = rot.getOperation();
            LLVM_DEBUG(llvm::dbgs() << "[RotFold] Checking op: " << op->getName() << " @" << op->getLoc() << "\n");

            // Must have a target operand that comes from a previous rotation
            Value target2 = rot.getTarget();
            Operation *prevOp = target2 ? target2.getDefiningOp() : nullptr;
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

            // Same axis?
            if (prevRot.getAxis() != rot.getAxis()) {
                LLVM_DEBUG(llvm::dbgs() << "[RotFold]   -> Skip: axis mismatch\n");
                return failure();
            }

            // Same control set (exact same Values in same order)
            auto prevCtrls = prevRot.getControls();
            auto ctrls = rot.getControls();
            if (prevCtrls.size() != ctrls.size()) {
                LLVM_DEBUG(llvm::dbgs() << "[RotFold]   -> Skip: control arity mismatch\n");
                return failure();
            }
            for (auto [a, b] : llvm::zip(prevCtrls, ctrls)) {
                if (a != b) {
                    LLVM_DEBUG(llvm::dbgs() << "[RotFold]   -> Skip: control mismatch\n");
                    return failure();
                }
            }

            // The result of prevOp (its target out) must be used only by op (strict adjacency)
            if (prevOp->getNumResults() != 1) {
                LLVM_DEBUG(llvm::dbgs() << "[RotFold]   -> Skip: prevOp not single-result\n");
                return failure();
            }
            Value out1 = prevOp->getResult(0);
            if (!out1.hasOneUse()) {
                LLVM_DEBUG(llvm::dbgs() << "[RotFold]   -> Skip: previous result has extra uses\n");
                return failure();
            }

            // Angles must be constants we can add
            auto a1Attr = dyn_cast_or_null<FloatAttr>(prevRot.getAngleAttr());
            auto a2Attr = dyn_cast_or_null<FloatAttr>(rot.getAngleAttr());
            if (!a1Attr || !a2Attr) {
                LLVM_DEBUG(llvm::dbgs() << "[RotFold]   -> Skip: non-constant angles\n");
                return failure();
            }

            APFloat sum = a1Attr.getValue();
            sum.add(a2Attr.getValue(), APFloat::rmNearestTiesToEven);

            // Find index of the target operand inside op to rewire it past prevOp.
            // Prefer an interface accessor if you have one; otherwise, search.
            int targetIdx = -1;
            for (int i = 0, e = (int) op->getNumOperands(); i < e; ++i) {
                if (op->getOperand(i) == out1) {
                    targetIdx = i;
                    break;
                }
            }
            if (targetIdx < 0) {
                LLVM_DEBUG(llvm::dbgs() << "[RotFold]   -> Skip: could not locate target operand index\n");
                return failure();
            }

            // Type sanity: result type of prevOp input == result type of op result
            // We assume single-result rotations with same qubit type; skip if not needed.

            LLVM_DEBUG(llvm::dbgs() << "[RotFold]    Match: folding " << prevOp->getName() << " + " << op->getName()
                                    << " (same axis, same controls)\n");

            // Update op angle in-place
            auto newAngleAttr = FloatAttr::get(a2Attr.getType(), sum);
            rot.setAngleAttr(newAngleAttr);

            // Rewire op's target operand to prevOp's *input target*
            //    (skip prevRot) i.e., fold into op and bypass prevOp.
            Value in1Target = prevRot.getTarget(); // prevRot's input target
            // If prevRot.getTarget() returns the *input*, good. If it returns the *output*,
            // use prevRot.getOperation()->getOperand(<idx-of-target-input>) instead.
            // For safety, derive from operand equality:
            //   prevOp has 1 result (out1). Its single target input is some operand of prevOp.
            // We'll pick the unique operand of prevOp that has the same type as out1 and is a qubit.
            // But since you have getTarget(), we'll rely on it:
            op->setOperand(targetIdx, in1Target);

            // Erase prevOp (its result was only used by op)
            rewriter.eraseOp(prevOp);

            LLVM_DEBUG({
                llvm::dbgs() << "[RotFold]      new angle set, rewired operand #" << targetIdx
                             << " to bypass previous rotation\n";
            });
            LLVM_DEBUG(llvm::dbgs() << "[RotFold]   Fold complete\n");
            return success();
        }
    };

    class QNetPeepholeOptimizationsPass : public impl::QNetPeepholeOptimizationsBase<QNetPeepholeOptimizationsPass> {
        using QNetPeepholeOptimizationsBase::QNetPeepholeOptimizationsBase;
        void runOnOperation() override;
    };

    void QNetPeepholeOptimizationsPass::runOnOperation() {
        LLVM_DEBUG(llvm::dbgs() << "[QNet][Peephole Optimizations] starts, hermitianCancel=" << this->hermitianCancel
                                << ", rotationFold=" << this->rotationFolding << "\n");

        RewritePatternSet patterns(&getContext());
        if (this->hermitianCancel) {
            patterns.add<CancelHermitianPairPattern>(&getContext());
        }
        if (this->rotationFolding) {
            patterns.add<FoldRotationPairPattern>(&getContext());
        }

        GreedyRewriteConfig cfg;
        (void) applyPatternsAndFoldGreedily(getOperation(), std::move(patterns), cfg);

        LLVM_DEBUG(llvm::dbgs() << "[QNet][Peephole] finished\n");
    }

} // namespace qoala::analysis
