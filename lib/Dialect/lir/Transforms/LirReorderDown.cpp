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
#include "Dialect/lir/Passes.h.inc"

} // end namespace mlir

#include "Dialect/lir/LirOps.h"

using namespace mlir;
using namespace mlir::lir;

class LirReorderDownPass : public LirReorderDownPassBase<LirReorderDownPass>
{
    void runOnFunction() override;
};

template <typename TyOp>
struct LirReorderDownPattern : public OpRewritePattern<TyOp>
{
public:
    using OpRewritePattern<TyOp>::OpRewritePattern;
};

bool isClassicalOp(Operation *op)
{
    return isa<SendCMsgOp>(op) || isa<RecvCMsgOp>(op) || isa<NewCValueCOp>(op) || isa<CValueCtoQOp>(op);
}

bool isLocalClassicalOp(Operation *op)
{
    return isa<NewCValueCOp>(op) || isa<CValueCtoQOp>(op);
}

struct AllocReorderDownPattern : public LirReorderDownPattern<AllocateOp>
{
public:
    using LirReorderDownPattern<AllocateOp>::LirReorderDownPattern;

    LogicalResult matchAndRewrite(AllocateOp op, PatternRewriter &rewriter) const override
    {
        auto nextNode = op->getNextNode();
        // llvm::outs() << "next node = " << *nextNode << "\n";
        if (!nextNode)
        {
            return failure();
        }
        for (auto operand : nextNode->getOperands())
        {
            if (op == operand.getDefiningOp())
            {
                return failure();
            }
        }

        if (isClassicalOp(nextNode))
        {
            // llvm::outs() << "next node is classical\n";
            rewriter.setInsertionPointAfter(nextNode);
            auto newOp = rewriter.create<AllocateOp>(op->getLoc(), QubitType::get(op->getContext()));
            rewriter.replaceOp(op, newOp.getResult());
            return success();
        }
        return failure();
    }
};

struct EntangleReorderDownPattern : public LirReorderDownPattern<EntangleOp>
{
public:
    using LirReorderDownPattern<EntangleOp>::LirReorderDownPattern;

    LogicalResult matchAndRewrite(EntangleOp op, PatternRewriter &rewriter) const override
    {
        auto nextNode = op->getNextNode();
        // llvm::outs() << "next node = " << *nextNode << "\n";
        if (!nextNode)
        {
            return failure();
        }
        for (auto operand : nextNode->getOperands())
        {
            if (op == operand.getDefiningOp())
            {
                return failure();
            }
        }

        if (isLocalClassicalOp(nextNode))
        {
            // llvm::outs() << "next node is classical\n";
            rewriter.setInsertionPointAfter(nextNode);
            auto newOp = rewriter.create<EntangleOp>(op->getLoc(), QubitType::get(op->getContext()));
            rewriter.replaceOp(op, newOp.getResult());
            return success();
        }
        return failure();
    }
};

struct RotXReorderDownPattern : public LirReorderDownPattern<RotXOp>
{
public:
    using LirReorderDownPattern<RotXOp>::LirReorderDownPattern;

    LogicalResult matchAndRewrite(RotXOp op, PatternRewriter &rewriter) const override
    {
        auto nextNode = op->getNextNode();
        if (!nextNode)
        {
            return failure();
        }
        for (auto operand : nextNode->getOperands())
        {
            if (op == operand.getDefiningOp())
            {
                return failure();
            }
        }

        if (isClassicalOp(nextNode))
        {
            rewriter.setInsertionPointAfter(nextNode);
            auto newOp = rewriter.create<RotXOp>(op->getLoc(), QubitType::get(op->getContext()), op.qin(), op.cin());
            rewriter.replaceOp(op, newOp.getResult());
            return success();
        }
        return failure();
    }
};

struct RotYReorderDownPattern : public LirReorderDownPattern<RotYOp>
{
public:
    using LirReorderDownPattern<RotYOp>::LirReorderDownPattern;

    LogicalResult matchAndRewrite(RotYOp op, PatternRewriter &rewriter) const override
    {
        auto nextNode = op->getNextNode();
        if (!nextNode)
        {
            return failure();
        }
        for (auto operand : nextNode->getOperands())
        {
            if (op == operand.getDefiningOp())
            {
                return failure();
            }
        }

        if (isClassicalOp(nextNode))
        {
            rewriter.setInsertionPointAfter(nextNode);
            auto newOp = rewriter.create<RotYOp>(op->getLoc(), QubitType::get(op->getContext()), op.qin(), op.cin());
            rewriter.replaceOp(op, newOp.getResult());
            return success();
        }
        return failure();
    }
};

struct RotZReorderDownPattern : public LirReorderDownPattern<RotZOp>
{
public:
    using LirReorderDownPattern<RotZOp>::LirReorderDownPattern;

    LogicalResult matchAndRewrite(RotZOp op, PatternRewriter &rewriter) const override
    {
        auto nextNode = op->getNextNode();
        if (!nextNode)
        {
            return failure();
        }
        for (auto operand : nextNode->getOperands())
        {
            if (op == operand.getDefiningOp())
            {
                return failure();
            }
        }

        if (isClassicalOp(nextNode))
        {
            rewriter.setInsertionPointAfter(nextNode);
            auto newOp = rewriter.create<RotZOp>(op->getLoc(), QubitType::get(op->getContext()), op.qin(), op.cin());
            rewriter.replaceOp(op, newOp.getResult());
            return success();
        }
        return failure();
    }
};

struct GateXReorderDownPattern : public LirReorderDownPattern<GateXOp>
{
public:
    using LirReorderDownPattern<GateXOp>::LirReorderDownPattern;

    LogicalResult matchAndRewrite(GateXOp op, PatternRewriter &rewriter) const override
    {
        auto nextNode = op->getNextNode();
        if (!nextNode)
        {
            return failure();
        }
        for (auto operand : nextNode->getOperands())
        {
            if (op == operand.getDefiningOp())
            {
                return failure();
            }
        }

        if (isClassicalOp(nextNode))
        {
            rewriter.setInsertionPointAfter(nextNode);
            auto newOp = rewriter.create<GateXOp>(op->getLoc(), QubitType::get(op->getContext()), op.qin());
            rewriter.replaceOp(op, newOp.getResult());
            return success();
        }
        return failure();
    }
};

struct GateYReorderDownPattern : public LirReorderDownPattern<GateYOp>
{
public:
    using LirReorderDownPattern<GateYOp>::LirReorderDownPattern;

    LogicalResult matchAndRewrite(GateYOp op, PatternRewriter &rewriter) const override
    {
        auto nextNode = op->getNextNode();
        if (!nextNode)
        {
            return failure();
        }
        for (auto operand : nextNode->getOperands())
        {
            if (op == operand.getDefiningOp())
            {
                return failure();
            }
        }

        if (isClassicalOp(nextNode))
        {
            rewriter.setInsertionPointAfter(nextNode);
            auto newOp = rewriter.create<GateYOp>(op->getLoc(), QubitType::get(op->getContext()), op.qin());
            rewriter.replaceOp(op, newOp.getResult());
            return success();
        }
        return failure();
    }
};

struct GateZReorderDownPattern : public LirReorderDownPattern<GateZOp>
{
public:
    using LirReorderDownPattern<GateZOp>::LirReorderDownPattern;

    LogicalResult matchAndRewrite(GateZOp op, PatternRewriter &rewriter) const override
    {
        auto nextNode = op->getNextNode();
        if (!nextNode)
        {
            return failure();
        }
        for (auto operand : nextNode->getOperands())
        {
            if (op == operand.getDefiningOp())
            {
                return failure();
            }
        }

        if (isClassicalOp(nextNode))
        {
            rewriter.setInsertionPointAfter(nextNode);
            auto newOp = rewriter.create<GateZOp>(op->getLoc(), QubitType::get(op->getContext()), op.qin());
            rewriter.replaceOp(op, newOp.getResult());
            return success();
        }
        return failure();
    }
};

struct GateHReorderDownPattern : public LirReorderDownPattern<GateHOp>
{
public:
    using LirReorderDownPattern<GateHOp>::LirReorderDownPattern;

    LogicalResult matchAndRewrite(GateHOp op, PatternRewriter &rewriter) const override
    {
        auto nextNode = op->getNextNode();
        if (!nextNode)
        {
            return failure();
        }
        for (auto operand : nextNode->getOperands())
        {
            if (op == operand.getDefiningOp())
            {
                return failure();
            }
        }

        if (isClassicalOp(nextNode))
        {
            rewriter.setInsertionPointAfter(nextNode);
            auto newOp = rewriter.create<GateHOp>(op->getLoc(), QubitType::get(op->getContext()), op.qin());
            rewriter.replaceOp(op, newOp.getResult());
            return success();
        }
        return failure();
    }
};

struct GateCphaseReorderDownPattern : public LirReorderDownPattern<GateCphaseOp>
{
public:
    using LirReorderDownPattern<GateCphaseOp>::LirReorderDownPattern;

    LogicalResult matchAndRewrite(GateCphaseOp op, PatternRewriter &rewriter) const override
    {
        auto nextNode = op->getNextNode();
        if (!nextNode)
        {
            return failure();
        }
        for (auto operand : nextNode->getOperands())
        {
            if (op == operand.getDefiningOp())
            {
                return failure();
            }
        }

        if (isClassicalOp(nextNode))
        {
            rewriter.setInsertionPointAfter(nextNode);
            auto newOp = rewriter.create<GateCphaseOp>(op->getLoc(), QubitType::get(op->getContext()), QubitType::get(op->getContext()), op.qin0(), op.qin1());
            rewriter.replaceOp(op, newOp.getResults());
            return success();
        }
        return failure();
    }
};

struct GateCnotReorderDownPattern : public LirReorderDownPattern<GateCnotOp>
{
public:
    using LirReorderDownPattern<GateCnotOp>::LirReorderDownPattern;

    LogicalResult matchAndRewrite(GateCnotOp op, PatternRewriter &rewriter) const override
    {
        auto nextNode = op->getNextNode();
        if (!nextNode)
        {
            return failure();
        }
        for (auto operand : nextNode->getOperands())
        {
            if (op == operand.getDefiningOp())
            {
                return failure();
            }
        }

        if (isClassicalOp(nextNode))
        {
            rewriter.setInsertionPointAfter(nextNode);
            auto newOp = rewriter.create<GateCnotOp>(op->getLoc(), QubitType::get(op->getContext()), QubitType::get(op->getContext()), op.qin0(), op.qin1());
            rewriter.replaceOp(op, newOp.getResults());
            return success();
        }
        return failure();
    }
};

struct GateMeasReorderDownPattern : public LirReorderDownPattern<MeasOp>
{
public:
    using LirReorderDownPattern<MeasOp>::LirReorderDownPattern;

    LogicalResult matchAndRewrite(MeasOp op, PatternRewriter &rewriter) const override
    {
        auto nextNode = op->getNextNode();
        if (!nextNode)
        {
            return failure();
        }
        for (auto operand : nextNode->getOperands())
        {
            if (op == operand.getDefiningOp())
            {
                return failure();
            }
        }

        if (isClassicalOp(nextNode))
        {
            rewriter.setInsertionPointAfter(nextNode);
            auto newOp = rewriter.create<MeasOp>(op->getLoc(), CValueQType::get(op->getContext()), op.qin());
            rewriter.replaceOp(op, newOp.getResult());
            return success();
        }
        return failure();
    }
};

struct NewCValueQReorderDownPattern : public LirReorderDownPattern<NewCValueQOp>
{
public:
    using LirReorderDownPattern<NewCValueQOp>::LirReorderDownPattern;

    LogicalResult matchAndRewrite(NewCValueQOp op, PatternRewriter &rewriter) const override
    {
        auto nextNode = op->getNextNode();
        if (!nextNode)
        {
            return failure();
        }
        for (auto operand : nextNode->getOperands())
        {
            if (op == operand.getDefiningOp())
            {
                return failure();
            }
        }

        if (isClassicalOp(nextNode))
        {
            rewriter.setInsertionPointAfter(nextNode);
            auto newOp = rewriter.create<NewCValueQOp>(op->getLoc(), CValueQType::get(op->getContext()));
            rewriter.replaceOp(op, newOp.getResult());
            return success();
        }
        return failure();
    }
};

struct CValueQtoCReorderDownPattern : public LirReorderDownPattern<CValueQtoCOp>
{
public:
    using LirReorderDownPattern<CValueQtoCOp>::LirReorderDownPattern;

    LogicalResult matchAndRewrite(CValueQtoCOp op, PatternRewriter &rewriter) const override
    {
        auto nextNode = op->getNextNode();
        if (!nextNode)
        {
            return failure();
        }
        for (auto operand : nextNode->getOperands())
        {
            if (op == operand.getDefiningOp())
            {
                return failure();
            }
        }

        if (isClassicalOp(nextNode))
        {
            rewriter.setInsertionPointAfter(nextNode);
            auto newOp = rewriter.create<CValueQtoCOp>(op->getLoc(), CValueCType::get(op->getContext()), op.cin());
            rewriter.replaceOp(op, newOp.getResult());
            return success();
        }
        return failure();
    }
};

void LirReorderDownPass::runOnFunction()
{
    OwningRewritePatternList patterns(&getContext());

    patterns.add<
        AllocReorderDownPattern,
        EntangleReorderDownPattern,
        RotXReorderDownPattern,
        RotYReorderDownPattern,
        RotZReorderDownPattern,
        GateXReorderDownPattern,
        GateYReorderDownPattern,
        GateZReorderDownPattern,
        GateHReorderDownPattern,
        GateCphaseReorderDownPattern,
        GateCnotReorderDownPattern,
        GateMeasReorderDownPattern,
        NewCValueQReorderDownPattern,
        CValueQtoCReorderDownPattern>(&getContext());

    auto config = GreedyRewriteConfig();
    config.maxIterations = 10;

    if (failed(
            applyPatternsAndFoldGreedily(getFunction(), std::move(patterns), config)))
    {
        llvm::outs() << "failed!\n";
        signalPassFailure();
    }
}

namespace mlir
{

    std::unique_ptr<FunctionPass> createLirReorderDownPass()
    {
        return std::make_unique<LirReorderDownPass>();
    }

} // namespace mlir
