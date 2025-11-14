#include "Dialect/Helpers/MIRToLIRHelperPasses.h"
#include "Dialect/QMem/QMem.h"
#include "Dialect/QNet/Passes.h"
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

    static void populateConversionTarget(ConversionTarget &target) {
        // When unfolding classical communication ops, all the ops in QMem are legal
        target.addLegalDialect<QMemDialect>();
        // ... EXCEPT the plural versions of them
        target.addIllegalOp<SendIntsOp, SendFloatsOp, RecvIntsOp, RecvFloatsOp>();
        // TODO - Check if call ops ARE ALSO allowed in this conversion
        //target.addLegalOp<func::CallOp>();
    }

    struct UnfoldSendIntsPattern : OpRewritePattern<SendIntsOp> {
        using OpRewritePattern::OpRewritePattern;

        LogicalResult matchAndRewrite(SendIntsOp op, PatternRewriter &rewriter) const override {
            // TODO - Implement the logic for the lower
            return failure();
        }
    };

    class UnfoldClassicalCommOpsPass : public impl::UnfoldClassicalCommOpsBase<UnfoldClassicalCommOpsPass> {
    public:
        using UnfoldClassicalCommOpsBase::UnfoldClassicalCommOpsBase;
        void runOnOperation() override;
    };

    void UnfoldClassicalCommOpsPass::runOnOperation() {
        LLVM_DEBUG(llvm::dbgs() << "[QMem][Unfold comm ops] starts\n");
        const FuncOp funcOp = this->getOperation();
        MLIRContext &context = this->getContext();

        ConversionTarget unfoldTarget(context);
        RewritePatternSet patterns(&context);

        populateConversionTarget(unfoldTarget);
        patterns.add<UnfoldSendIntsPattern>(&context);

        (void) applyPartialConversion(funcOp, unfoldTarget, std::move(patterns));
    }
}