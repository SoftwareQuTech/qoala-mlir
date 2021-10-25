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
#include "Dialect/lircommon/Passes.h.inc"

} // end namespace mlir

#include "Dialect/lircommon/LirCommonOps.h"

using namespace mlir;
using namespace mlir::lircommon;

class LirCommonRewritePass : public LirCommonRewritePassBase<LirCommonRewritePass>
{
    void runOnFunction() override;
};

namespace
{
#include "Dialect/lircommon/Transforms/LirCommonRewrites.h.inc"
} // namespace

void LirCommonRewritePass::runOnFunction()
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

    std::unique_ptr<FunctionPass> createLirCommonRewritePass()
    {
        return std::make_unique<LirCommonRewritePass>();
    }

} // namespace mlir
