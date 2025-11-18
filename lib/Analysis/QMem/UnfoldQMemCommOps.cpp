#include <mlir/Transforms/GreedyPatternRewriteDriver.h>

#include "Analysis/QMem/Unfold.h"
#include "Dialect/Helpers/MIRToLIRHelperPasses.h"
#include "Dialect/QMem/QMem.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Transforms/DialectConversion.h"

#include "llvm/Support/Debug.h"

using namespace mlir;
using namespace qoala::dialects::qmem;

#define DEBUG_TYPE "qmem-unfold-qmem-ops"

namespace qoala::analysis {
#define GEN_PASS_DEF_UNFOLDCLASSICALCOMMOPS
#include "Dialect/Helpers/HelperPasses.h.inc"

    using UnfoldSendIntsPattern = unfold::UnfoldSendPattern<SendIntsOp, SendIntOp, IntegerAttr>;
    using UnfoldSendFloatsPattern = unfold::UnfoldSendPattern<SendFloatsOp, SendFloatOp, FloatAttr>;

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
        patterns.add<UnfoldSendIntsPattern>(&context);//, IntegerType::get(&context, 32));
        patterns.add<UnfoldSendFloatsPattern>(&context);//, Float32Type::getF32(&context));

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