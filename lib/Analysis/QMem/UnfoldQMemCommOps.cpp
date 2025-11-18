#include <mlir/Transforms/GreedyPatternRewriteDriver.h>

#include "Dialect/Helpers/MIRToLIRHelperPasses.h"
#include "Dialect/QMem/QMem.h"
#include "llvm/ADT/TypeSwitch.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Transforms/DialectConversion.h"

#include "llvm/Support/Debug.h"

using namespace mlir;
using namespace qoala::dialects::qmem;

#define DEBUG_TYPE "qmem-unfold-qmem-ops"

namespace qoala::analysis {
#define GEN_PASS_DEF_UNFOLDCLASSICALCOMMOPS
#include "Dialect/Helpers/HelperPasses.h.inc"

    struct UnfoldSendIntsPattern : OpRewritePattern<SendIntsOp> {
        using OpRewritePattern::OpRewritePattern;

        LogicalResult matchAndRewrite(SendIntsOp sendIntsOp, PatternRewriter &rewriter) const override {
            // TODO - Generalize this to apply to SendIntsOp and SendFloatsOp
            //  (should not be difficult with a template)
            // For a multi-value send comm op, we expect an "assembler" op, which packs the values
            // in a tensor/vector.
            Operation *originalOp = sendIntsOp.getCin().getDefiningOp();
            std::vector<Operation *> opsToDelete;
            (void) llvm::TypeSwitch<Operation *, LogicalResult>(originalOp)
                .Case([&](arith::ConstantOp constantOp) -> LogicalResult {
                    // If the values are all constants, then we expect the tensor/vector is
                    // assembled using an arith.constant dense operation
                    const TypedAttr value = constantOp.getValue();
                    assert(isa<DenseIntElementsAttr>(value) &&
                        "[QMem - Unfold comm ops] arith.constant does not use a dense attribute argument.");
                    for (APInt eso : dyn_cast<DenseIntElementsAttr>(value)) {
                        auto newIntVal = rewriter.create<arith::ConstantOp>(
                            constantOp.getLoc(), rewriter.getIntegerAttr(rewriter.getI32Type(), eso)
                            );
                        rewriter.create<SendIntOp>(constantOp.getLoc(), newIntVal, sendIntsOp.getRemote());
                    }
                    if (constantOp->hasOneUse()) {
                        // If the constantOp has one use, it *must* be sendIntsOp. If so, delete it
                        assert(*constantOp->getUsers().begin() == sendIntsOp &&
                            "[QMem - Unfold comm ops] The single usage of arith.constant is not the op being deleted.");
                        opsToDelete.push_back(constantOp);
                    }
                    return success();
                })
                // TODO - Figure out how runtime values ar packed into a Tensor/Vector
                .Default([](Operation *operation) -> LogicalResult {
                    return operation->emitOpError("Unknown way to translate a QRemote operation to iQoala: '") << *operation << "'\n";
                });
            // First, delete the leaf operation
            rewriter.eraseOp(sendIntsOp);
            // Then any operations that yield a value used exclusively by the leaf operation
            // TODO - Do this recursively if necessary
            for (const auto opToDelete : opsToDelete) {
                rewriter.eraseOp(opToDelete);
            }
            return success(true);
        }
    };

    struct UnfoldSendFloatsPattern : OpRewritePattern<SendFloatsOp> {
        using OpRewritePattern::OpRewritePattern;

        LogicalResult matchAndRewrite(SendFloatsOp sendFloatsOp, PatternRewriter &rewriter) const override {
            // TODO - Implement the logic for the lower
            // For a multi-value send comm op, we expect an "assembler" op, which packs the values
            // in a tensor/vector.
            Operation *originalOp = sendFloatsOp.getCin().getDefiningOp();
            const auto result = llvm::TypeSwitch<Operation *, LogicalResult>(originalOp)
                .Case([&](arith::ConstantOp constantOp) -> LogicalResult {
                    // If the values are all constants, then we expect the tensor/vector is
                    // assembled using an arith.constant dense operation
                    TypedAttr value = constantOp.getValue();
                    return success();
                })
                .Default([](Operation *operation) -> LogicalResult {
                    return operation->emitOpError("Unknown way to translate a QRemote operation to iQoala: '") << *operation << "'\n";
                });
            return result;
        }
    };

    class UnfoldClassicalCommOpsPass : public impl::UnfoldClassicalCommOpsBase<UnfoldClassicalCommOpsPass> {
    public:
        using UnfoldClassicalCommOpsBase::UnfoldClassicalCommOpsBase;
        void runOnOperation() override;
    };

    void UnfoldClassicalCommOpsPass::runOnOperation() {
        LLVM_DEBUG(llvm::dbgs() << "[QMem - Unfold comm ops] starts\n");
        const FuncOp funcOp = this->getOperation();
        MLIRContext &context = this->getContext();

        RewritePatternSet patterns(&context);
        patterns.add<UnfoldSendIntsPattern, UnfoldSendFloatsPattern>(&context);

        GreedyRewriteConfig cfg;
        // If we don't disable region simplification, the folding of the code will be more aggressive
        // simplifying entire blocks if possible:
        // E.g, in test/Conversion/MIRtoLIR/BlkMeta/MIRtoLIR-precedences-predecessors.mlir
        // the aggressive folding will merge both "then" and "else" blocks into a single one,
        // inserting a block argument. That makes the test fail, since it expects 4 blocks, not 3.
        cfg.enableRegionSimplification = false;

        if (failed(applyPatternsAndFoldGreedily(funcOp, std::move(patterns), cfg))) {
            signalPassFailure();
        }
    }
}