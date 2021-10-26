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
        // llvm::outs() << "hello?\n";
        // sendOp->def
        // auto value = sendOp.getOperand();
        // llvm::outs() << "value = " << value << "\n";

        // auto parent = sendOp->getParentOp();
        // llvm::outs() << "parent = " << *parent << "\n";

        // auto nextNode = sendOp->getNextNode();
        // llvm::outs() << "next node = " << *nextNode << "\n";

        auto prevNode = sendOp->getPrevNode();
        // llvm::outs() << "prev node = " << *prevNode << "\n";

        auto prevIsAlloc = isa<AllocateOp>(prevNode);
        // llvm::outs() << "previous is alloc : " << prevIsAlloc << "\n";

        if (prevIsAlloc)
        {
            rewriter.setInsertionPoint(prevNode);
            rewriter.create<SendCMsgOp>(prevNode->getLoc(), sendOp.cin());
            rewriter.eraseOp(sendOp);
            return success();
        }
        else
        {
            return failure(); // THIS IS FUCKING NEEDED OTHERWISE PATTERN REWRITE WILL FAILLL!!!!??!?!?!?!?
        }

        // rewriter.replaceOp(sendOp, newSendOp.getResult());

        // rewriter.eraseOp(sendOp);
    }
};

struct GateXPattern : public LirRewritePattern<GateXOp>
{
public:
    using LirRewritePattern<GateXOp>::LirRewritePattern;

    LogicalResult matchAndRewrite(GateXOp op, PatternRewriter &rewriter) const override
    {
        return failure();
    }
};

struct ThisOneWorks : public ::mlir::RewritePattern
{
    ThisOneWorks(::mlir::MLIRContext *context)
        : ::mlir::RewritePattern("lircommon.gate_x", 1, context, {"lircommon.gate_y"}) {}
    ::mlir::LogicalResult matchAndRewrite(::mlir::Operation *op0,
                                          ::mlir::PatternRewriter &rewriter) const override
    {
        // Variables for capturing values and attributes used while creating ops
        ::mlir::Operation::operand_range c(op0->getOperands());
        ::mlir::Operation::operand_range q(op0->getOperands());

        // Match
        auto castedOp0 = ::llvm::dyn_cast<::mlir::lircommon::GateXOp>(op0);
        q = castedOp0.getODSOperands(0);
        c = castedOp0.getODSOperands(1);

        // Rewrite
        auto odsLoc = op0->getLoc();
        GateYOp tblgen_GateYOp_0 = rewriter.create<::mlir::lircommon::GateYOp>(odsLoc, QubitType::get(op0->getContext()), castedOp0.qin(), castedOp0.cin());

        rewriter.replaceOp(op0, tblgen_GateYOp_0.getResult());
        return ::mlir::success();
    };
};

void LirCommonRewritePass::runOnFunction()
{
    OwningRewritePatternList patterns(&getContext());
    // populateWithGenerated(patterns);

    patterns.add<SendRewritePattern>(&getContext());
    // patterns.add<GateXPattern>(&getContext());
    // patterns.add<ThisOneWorks>(&getContext());

    auto config = GreedyRewriteConfig();
    config.maxIterations = 2;

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
