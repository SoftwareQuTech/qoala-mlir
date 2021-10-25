#include "mlir/Dialect/StandardOps/IR/Ops.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "mlir/IR/Value.h"

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

template <typename TyOp>
struct LirRewritePattern : public OpRewritePattern<TyOp>
{
public:
    using OpRewritePattern<TyOp>::OpRewritePattern;
};

struct SendRewritePattern : public LirRewritePattern<SendCMsgOp>
{
public:
    using LirRewritePattern<SendCMsgOp>::LirRewritePattern;

    LogicalResult matchAndRewrite(SendCMsgOp sendOp, PatternRewriter &rewriter) const override
    {
        llvm::outs() << "hello?\n";
        // sendOp->def
        auto value = sendOp.getOperand();
        // llvm::outs() << "value = " << value << "\n";
        auto defining = value.getDefiningOp();
        // value.getUses()
        // llvm::outs() << "defining = " << *defining << "\n";

        auto parent = sendOp->getParentOp();
        // llvm::outs() << "parent = " << *parent << "\n";

        auto parentNextNode = parent->getNextNode();
        // llvm::outs() << "parent next node = " << parentNextNode << "\n";

        auto nextNode = sendOp->getNextNode();
        // llvm::outs() << "next node = " << *nextNode << "\n";

        auto prevNode = sendOp->getPrevNode();
        // llvm::outs() << "prev node = " << *prevNode << "\n";

        auto prevIsAlloc = isa<AllocateOp>(prevNode);
        // llvm::outs() << "previous is alloc : " << prevIsAlloc << "\n";

        if (prevIsAlloc)
        {
            // sendOp->moveBefore(prevNode);
            rewriter.create<SendCMsgOp>(prevNode->getLoc(), sendOp.cin());
            // rewriter.create<AllocateOp>(sendOp->getLoc(), QubitType::get(sendOp->getContext()));
            // rewriter.eraseOp(prevNode);
            rewriter.eraseOp(sendOp);
        }

        // rewriter.eraseOp(sendOp);

        return success();
    }
};

void LirCommonRewritePass::runOnFunction()
{
    OwningRewritePatternList patterns(&getContext());
    // populateWithGenerated(patterns);

    patterns.add<SendRewritePattern>(&getContext());

    auto config = GreedyRewriteConfig();
    config.maxIterations = 1;

    if (failed(
            applyPatternsAndFoldGreedily(getFunction(), std::move(patterns), config)))
    {
        llvm::outs() << "failed!\n";
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
