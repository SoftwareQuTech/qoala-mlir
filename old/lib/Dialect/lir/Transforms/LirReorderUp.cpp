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

class LirReorderUpPass : public LirReorderUpPassBase<LirReorderUpPass>
{
    void runOnFunction() override;
};

template <typename TyOp>
struct LirReorderUpPattern : public OpRewritePattern<TyOp>
{
public:
    using OpRewritePattern<TyOp>::OpRewritePattern;
};

bool isClassicalOp_forUp(Operation *op)
{
    return isa<SendCMsgOp>(op) || isa<RecvCMsgOp>(op) || isa<NewCValueCOp>(op) || isa<CValueCtoQOp>(op) || isa<CValueQtoCOp>(op);
}

bool isLocalClassicalOp_forUp(Operation *op)
{
    return isa<NewCValueCOp>(op) || isa<CValueCtoQOp>(op);
}

struct AllocReorderUpPattern : public LirReorderUpPattern<AllocateOp>
{
public:
    using LirReorderUpPattern<AllocateOp>::LirReorderUpPattern;

    LogicalResult matchAndRewrite(AllocateOp op, PatternRewriter &rewriter) const override
    {
        auto prevNode = op->getPrevNode();
        if (!prevNode)
        {
            return failure();
        }
        llvm::outs() << "prev node = " << *prevNode << "\n";

        if (isClassicalOp_forUp(prevNode))
        {
            llvm::outs() << "prev node is classical\n";
            rewriter.setInsertionPoint(prevNode);
            auto newOp = rewriter.create<AllocateOp>(op->getLoc(), QubitType::get(op->getContext()));
            rewriter.replaceOp(op, newOp.getResult());
            return success();
        }
        return failure();
    }
};

struct EntangleReorderUpPattern : public LirReorderUpPattern<EntangleOp>
{
public:
    using LirReorderUpPattern<EntangleOp>::LirReorderUpPattern;

    LogicalResult matchAndRewrite(EntangleOp op, PatternRewriter &rewriter) const override
    {
        auto prevNode = op->getPrevNode();
        if (!prevNode)
        {
            return failure();
        }
        // llvm::outs() << "next node = " << *prevNode << "\n";

        if (isLocalClassicalOp_forUp(prevNode))
        {
            // llvm::outs() << "next node is classical\n";
            rewriter.setInsertionPoint(prevNode);
            auto newOp = rewriter.create<EntangleOp>(op->getLoc(), QubitType::get(op->getContext()));
            rewriter.replaceOp(op, newOp.getResult());
            return success();
        }
        return failure();
    }
};

struct RotXReorderUpPattern : public LirReorderUpPattern<RotXOp>
{
public:
    using LirReorderUpPattern<RotXOp>::LirReorderUpPattern;

    LogicalResult matchAndRewrite(RotXOp op, PatternRewriter &rewriter) const override
    {
        auto prevNode = op->getPrevNode();
        if (!prevNode)
        {
            return failure();
        }
        for (auto operand : op.getOperands())
        {
            if (prevNode == operand.getDefiningOp())
            {
                return failure();
            }
        }

        if (isClassicalOp_forUp(prevNode))
        {
            rewriter.setInsertionPoint(prevNode);
            auto newOp = rewriter.create<RotXOp>(op->getLoc(), QubitType::get(op->getContext()), op.qin(), op.cin());
            rewriter.replaceOp(op, newOp.getResult());
            return success();
        }
        return failure();
    }
};

struct RotYReorderUpPattern : public LirReorderUpPattern<RotYOp>
{
public:
    using LirReorderUpPattern<RotYOp>::LirReorderUpPattern;

    LogicalResult matchAndRewrite(RotYOp op, PatternRewriter &rewriter) const override
    {
        auto prevNode = op->getPrevNode();
        if (!prevNode)
        {
            return failure();
        }
        for (auto operand : op.getOperands())
        {
            if (prevNode == operand.getDefiningOp())
            {
                return failure();
            }
        }

        if (isClassicalOp_forUp(prevNode))
        {
            rewriter.setInsertionPoint(prevNode);
            auto newOp = rewriter.create<RotYOp>(op->getLoc(), QubitType::get(op->getContext()), op.qin(), op.cin());
            rewriter.replaceOp(op, newOp.getResult());
            return success();
        }
        return failure();
    }
};

struct RotZReorderUpPattern : public LirReorderUpPattern<RotZOp>
{
public:
    using LirReorderUpPattern<RotZOp>::LirReorderUpPattern;

    LogicalResult matchAndRewrite(RotZOp op, PatternRewriter &rewriter) const override
    {
        auto prevNode = op->getPrevNode();
        if (!prevNode)
        {
            return failure();
        }
        for (auto operand : op.getOperands())
        {
            if (prevNode == operand.getDefiningOp())
            {
                return failure();
            }
        }

        if (isClassicalOp_forUp(prevNode))
        {
            rewriter.setInsertionPoint(prevNode);
            auto newOp = rewriter.create<RotZOp>(op->getLoc(), QubitType::get(op->getContext()), op.qin(), op.cin());
            rewriter.replaceOp(op, newOp.getResult());
            return success();
        }
        return failure();
    }
};

struct GateXReorderUpPattern : public LirReorderUpPattern<GateXOp>
{
public:
    using LirReorderUpPattern<GateXOp>::LirReorderUpPattern;

    LogicalResult matchAndRewrite(GateXOp op, PatternRewriter &rewriter) const override
    {
        auto prevNode = op->getPrevNode();
        if (!prevNode)
        {
            return failure();
        }
        if (prevNode == op->getOperand(0).getDefiningOp())
        {
            return failure();
        }

        if (isClassicalOp_forUp(prevNode))
        {
            rewriter.setInsertionPoint(prevNode);
            auto newOp = rewriter.create<GateXOp>(op->getLoc(), QubitType::get(op->getContext()), op.qin());
            rewriter.replaceOp(op, newOp.getResult());
            return success();
        }
        return failure();
    }
};

struct GateYReorderUpPattern : public LirReorderUpPattern<GateYOp>
{
public:
    using LirReorderUpPattern<GateYOp>::LirReorderUpPattern;

    LogicalResult matchAndRewrite(GateYOp op, PatternRewriter &rewriter) const override
    {
        auto prevNode = op->getPrevNode();
        if (!prevNode)
        {
            return failure();
        }
        if (prevNode == op->getOperand(0).getDefiningOp())
        {
            return failure();
        }

        if (isClassicalOp_forUp(prevNode))
        {
            rewriter.setInsertionPoint(prevNode);
            auto newOp = rewriter.create<GateYOp>(op->getLoc(), QubitType::get(op->getContext()), op.qin());
            rewriter.replaceOp(op, newOp.getResult());
            return success();
        }
        return failure();
    }
};

struct GateZReorderUpPattern : public LirReorderUpPattern<GateZOp>
{
public:
    using LirReorderUpPattern<GateZOp>::LirReorderUpPattern;

    LogicalResult matchAndRewrite(GateZOp op, PatternRewriter &rewriter) const override
    {
        auto prevNode = op->getPrevNode();
        if (!prevNode)
        {
            return failure();
        }
        if (prevNode == op->getOperand(0).getDefiningOp())
        {
            return failure();
        }

        if (isClassicalOp_forUp(prevNode))
        {
            rewriter.setInsertionPoint(prevNode);
            auto newOp = rewriter.create<GateZOp>(op->getLoc(), QubitType::get(op->getContext()), op.qin());
            rewriter.replaceOp(op, newOp.getResult());
            return success();
        }
        return failure();
    }
};

struct GateHReorderUpPattern : public LirReorderUpPattern<GateHOp>
{
public:
    using LirReorderUpPattern<GateHOp>::LirReorderUpPattern;

    LogicalResult matchAndRewrite(GateHOp op, PatternRewriter &rewriter) const override
    {
        auto prevNode = op->getPrevNode();
        if (!prevNode)
        {
            return failure();
        }
        if (prevNode == op->getOperand(0).getDefiningOp())
        {
            return failure();
        }

        if (isClassicalOp_forUp(prevNode))
        {
            rewriter.setInsertionPoint(prevNode);
            auto newOp = rewriter.create<GateHOp>(op->getLoc(), QubitType::get(op->getContext()), op.qin());
            rewriter.replaceOp(op, newOp.getResult());
            return success();
        }
        return failure();
    }
};

struct GateCphaseReorderUpPattern : public LirReorderUpPattern<GateCphaseOp>
{
public:
    using LirReorderUpPattern<GateCphaseOp>::LirReorderUpPattern;

    LogicalResult matchAndRewrite(GateCphaseOp op, PatternRewriter &rewriter) const override
    {
        auto prevNode = op->getPrevNode();
        if (!prevNode)
        {
            return failure();
        }
        for (auto operand : op.getOperands())
        {
            if (prevNode == operand.getDefiningOp())
            {
                return failure();
            }
        }

        if (isClassicalOp_forUp(prevNode))
        {
            rewriter.setInsertionPoint(prevNode);
            auto newOp = rewriter.create<GateCphaseOp>(op->getLoc(), QubitType::get(op->getContext()), QubitType::get(op->getContext()), op.qin0(), op.qin1());
            rewriter.replaceOp(op, newOp.getResults());
            return success();
        }
        return failure();
    }
};

struct GateCnotReorderUpPattern : public LirReorderUpPattern<GateCnotOp>
{
public:
    using LirReorderUpPattern<GateCnotOp>::LirReorderUpPattern;

    LogicalResult matchAndRewrite(GateCnotOp op, PatternRewriter &rewriter) const override
    {
        auto prevNode = op->getPrevNode();
        if (!prevNode)
        {
            return failure();
        }
        for (auto operand : op.getOperands())
        {
            if (prevNode == operand.getDefiningOp())
            {
                return failure();
            }
        }

        if (isClassicalOp_forUp(prevNode))
        {
            rewriter.setInsertionPoint(prevNode);
            auto newOp = rewriter.create<GateCnotOp>(op->getLoc(), QubitType::get(op->getContext()), QubitType::get(op->getContext()), op.qin0(), op.qin1());
            rewriter.replaceOp(op, newOp.getResults());
            return success();
        }
        return failure();
    }
};

struct GateMeasReorderUpPattern : public LirReorderUpPattern<MeasOp>
{
public:
    using LirReorderUpPattern<MeasOp>::LirReorderUpPattern;

    LogicalResult matchAndRewrite(MeasOp op, PatternRewriter &rewriter) const override
    {
        auto prevNode = op->getPrevNode();
        if (!prevNode)
        {
            return failure();
        }
        if (prevNode == op.getOperand().getDefiningOp())
        {
            return failure();
        }

        if (isClassicalOp_forUp(prevNode))
        {
            rewriter.setInsertionPoint(prevNode);
            auto newOp = rewriter.create<MeasOp>(op->getLoc(), CValueQType::get(op->getContext()), op.qin());
            rewriter.replaceOp(op, newOp.getResult());
            return success();
        }
        return failure();
    }
};

struct NewCValueQReorderUpPattern : public LirReorderUpPattern<NewCValueQOp>
{
public:
    using LirReorderUpPattern<NewCValueQOp>::LirReorderUpPattern;

    LogicalResult matchAndRewrite(NewCValueQOp op, PatternRewriter &rewriter) const override
    {
        auto prevNode = op->getPrevNode();
        if (!prevNode)
        {
            return failure();
        }

        if (isClassicalOp_forUp(prevNode))
        {
            rewriter.setInsertionPoint(prevNode);
            auto newOp = rewriter.create<NewCValueQOp>(op->getLoc(), CValueQType::get(op->getContext()));
            rewriter.replaceOp(op, newOp.getResult());
            return success();
        }
        return failure();
    }
};

struct CValueQtoCReorderUpPattern : public LirReorderUpPattern<CValueQtoCOp>
{
public:
    using LirReorderUpPattern<CValueQtoCOp>::LirReorderUpPattern;

    LogicalResult matchAndRewrite(CValueQtoCOp op, PatternRewriter &rewriter) const override
    {
        auto prevNode = op->getPrevNode();
        if (!prevNode)
        {
            return failure();
        }
        if (prevNode == op.getOperand().getDefiningOp())
        {
            return failure();
        }

        if (isClassicalOp_forUp(prevNode))
        {
            rewriter.setInsertionPoint(prevNode);
            auto newOp = rewriter.create<CValueQtoCOp>(op->getLoc(), CValueCType::get(op->getContext()), op.cin());
            rewriter.replaceOp(op, newOp.getResult());
            return success();
        }
        return failure();
    }
};

void LirReorderUpPass::runOnFunction()
{
    OwningRewritePatternList patterns(&getContext());

    patterns.add<
        AllocReorderUpPattern,
        EntangleReorderUpPattern,
        RotXReorderUpPattern,
        RotYReorderUpPattern,
        RotZReorderUpPattern,
        GateXReorderUpPattern,
        GateYReorderUpPattern,
        GateZReorderUpPattern,
        GateHReorderUpPattern,
        GateCphaseReorderUpPattern,
        GateCnotReorderUpPattern,
        GateMeasReorderUpPattern,
        NewCValueQReorderUpPattern
        // CValueQtoCReorderUpPattern
        //
        >(&getContext());

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

    std::unique_ptr<FunctionPass> createLirReorderUpPass()
    {
        return std::make_unique<LirReorderUpPass>();
    }

} // namespace mlir
