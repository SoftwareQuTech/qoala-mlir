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
            Operation *prevOp = op->getOperand(0).getDefiningOp();
            if (!prevOp) {
                LLVM_DEBUG(llvm::dbgs() << "[HermitianCancel]   -> Skip: first operand has no defining op\n");
                return failure();
            }

            // prevOp must also implement the same interface
            auto prevOpIface = dyn_cast<HermitianOpIface>(prevOp);
            if (!prevOpIface) {
                LLVM_DEBUG(llvm::dbgs() << "[HermitianCancel]   -> Skip: previous op not Hermitian\n");
                return failure();
            }

            // Same op class/kind
            if (prevOp->getName() != op->getName()) {
                LLVM_DEBUG(llvm::dbgs() << "[HermitianCancel]   -> Skip: different op types (" << prevOp->getName()
                                        << " vs " << op->getName() << ")\n");
                return failure();
            }

            // Arity alignment
            if (prevOp->getNumResults() != op->getNumOperands() || prevOp->getNumOperands() != op->getNumResults()) {
                LLVM_DEBUG(llvm::dbgs() << "[HermitianCancel]   -> Skip: mismatched arity\n");
                return failure();
            }

            // Strict adjacency and single-use of prev results
            for (auto [opd, res] : llvm::zip(op->getOperands(), prevOp->getResults())) {
                if (opd != res) {
                    LLVM_DEBUG(llvm::dbgs() << "[HermitianCancel]   -> Skip: operands/results not aligned\n");
                    return failure();
                }
                if (!res.hasOneUse()) {
                    LLVM_DEBUG(llvm::dbgs() << "[HermitianCancel]   -> Skip: result has multiple uses\n");
                    return failure();
                }
            }

            // Rewrite: forward prevOp's inputs to replace op's outputs
            LLVM_DEBUG(llvm::dbgs() << "[HermitianCancel]   Matched pair: " << prevOp->getName() << " / "
                                    << op->getName() << " -> performing cancellation\n");

            for (auto [res2, opd1] : llvm::zip(op->getResults(), op->getOperands())) {
                rewriter.replaceAllUsesWith(res2, opd1);
                LLVM_DEBUG(llvm::dbgs() << "[HermitianCancel]      replaced uses of " << res2 << " with " << opd1
                                        << "\n");
            }

            LLVM_DEBUG(llvm::dbgs() << "[HermitianCancel]      erasing ops\n");
            rewriter.eraseOp(op);
            rewriter.eraseOp(prevOp);

            LLVM_DEBUG(llvm::dbgs() << "[HermitianCancel]   Cancellation complete\n");
            return success();
        }
    };

    class QNetPeepholeOptimizationsPass : public impl::QNetPeepholeOptimizationsBase<QNetPeepholeOptimizationsPass> {
        using QNetPeepholeOptimizationsBase::QNetPeepholeOptimizationsBase;
        void runOnOperation() override;
    };

    void QNetPeepholeOptimizationsPass::runOnOperation() {
        LLVM_DEBUG(llvm::dbgs() << "[QNet][Peephole Optimizations] starts, hermitianCancellations="
                                << this->hermiatianCancellations << "\n");

        RewritePatternSet patterns(&getContext());
        if (this->hermiatianCancellations) {
            patterns.add<CancelHermitianPairPattern>(&getContext());
            GreedyRewriteConfig cfg;
            (void) applyPatternsAndFoldGreedily(getOperation(), std::move(patterns), cfg);
        }

        LLVM_DEBUG(llvm::dbgs() << "[QNet][Peephole Optimizations] finished\n");
    }

} // namespace qoala::analysis
