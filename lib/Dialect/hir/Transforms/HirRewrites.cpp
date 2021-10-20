#include "mlir/Dialect/StandardOps/IR/Ops.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir
{

#define GEN_PASS_CLASSES
#include "Dialect/hir/Passes.h.inc"

} // end namespace mlir

#include "Dialect/hir/HirOps.h"

using namespace mlir;
using namespace mlir::hir;

class HirRewritePass : public HirRewritePassBase<HirRewritePass>
{
    void runOnFunction() override;
};

namespace
{
#include "Dialect/hir/Transforms/HirRewrites.h.inc"
} // namespace

void HirRewritePass::runOnFunction()
{
    OwningRewritePatternList patterns(&getContext());
    populateWithGenerated(patterns);
    if (failed(
            applyPatternsAndFoldGreedily(getFunction(), std::move(patterns))))
    {
        signalPassFailure();
    }
}

namespace mlir
{

    std::unique_ptr<FunctionPass> createHirRewritePass()
    {
        return std::make_unique<HirRewritePass>();
    }

} // namespace mlir
