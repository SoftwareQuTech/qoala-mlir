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

class LirRewritePass : public LirRewritePassBase<LirRewritePass>
{
    void runOnFunction() override;
};

namespace
{
#include "Dialect/lir/Transforms/LirRewrites.h.inc"
} // namespace

template <typename TyOp>
struct LirRewritePattern : public OpRewritePattern<TyOp>
{
public:
    using OpRewritePattern<TyOp>::OpRewritePattern;
};

struct SendReorderPattern : public LirRewritePattern<SendCMsgOp>
{
public:
    using LirRewritePattern<SendCMsgOp>::LirRewritePattern;

    LogicalResult matchAndRewrite(SendCMsgOp sendOp, PatternRewriter &rewriter) const override
    {
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
        return failure();
    }
};

bool isClassicalOp(Operation *op)
{
    return isa<SendCMsgOp>(op) || isa<RecvCMsgOp>(op) || isa<NewCValueCOp>(op) || isa<CValueCtoQOp>(op);
}

bool isLocalClassicalOp(Operation *op)
{
    return isa<NewCValueCOp>(op) || isa<CValueCtoQOp>(op);
}

struct AllocReorderPattern : public LirRewritePattern<AllocateOp>
{
public:
    using LirRewritePattern<AllocateOp>::LirRewritePattern;

    LogicalResult matchAndRewrite(AllocateOp op, PatternRewriter &rewriter) const override
    {
        auto nextNode = op->getNextNode();
        // llvm::outs() << "next node = " << *nextNode << "\n";

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

struct EntangleReorderPattern : public LirRewritePattern<EntangleOp>
{
public:
    using LirRewritePattern<EntangleOp>::LirRewritePattern;

    LogicalResult matchAndRewrite(EntangleOp op, PatternRewriter &rewriter) const override
    {
        auto nextNode = op->getNextNode();
        // llvm::outs() << "next node = " << *nextNode << "\n";

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

struct GateCphaseReorderPattern : public LirRewritePattern<GateCphaseOp>
{
public:
    using LirRewritePattern<GateCphaseOp>::LirRewritePattern;

    LogicalResult matchAndRewrite(GateCphaseOp op, PatternRewriter &rewriter) const override
    {
        auto nextNode = op->getNextNode();
        // llvm::outs() << "next node = " << *nextNode << "\n";

        if (isClassicalOp(nextNode))
        {
            // llvm::outs() << "next node is classical\n";
            rewriter.setInsertionPointAfter(nextNode);
            auto newOp = rewriter.create<GateCphaseOp>(op->getLoc(), QubitType::get(op->getContext()), QubitType::get(op->getContext()), op.qin0(), op.qin1());
            rewriter.replaceOp(op, newOp.getResults());
            return success();
        }
        return failure();
    }
};

struct RotXPattern : public LirRewritePattern<RotXOp>
{
public:
    using LirRewritePattern<RotXOp>::LirRewritePattern;

    LogicalResult matchAndRewrite(RotXOp op, PatternRewriter &rewriter) const override
    {
        return failure();
    }
};

struct ThisOneWorks : public ::mlir::RewritePattern
{
    ThisOneWorks(::mlir::MLIRContext *context)
        : ::mlir::RewritePattern("lir.rot_x", 1, context, {"lir.rot_y"}) {}
    ::mlir::LogicalResult matchAndRewrite(::mlir::Operation *op0,
                                          ::mlir::PatternRewriter &rewriter) const override
    {
        // Variables for capturing values and attributes used while creating ops
        ::mlir::Operation::operand_range c(op0->getOperands());
        ::mlir::Operation::operand_range q(op0->getOperands());

        // Match
        auto castedOp0 = ::llvm::dyn_cast<::mlir::lir::RotXOp>(op0);
        q = castedOp0.getODSOperands(0);
        c = castedOp0.getODSOperands(1);

        // Rewrite
        auto odsLoc = op0->getLoc();
        RotYOp tblgen_RotYOp_0 = rewriter.create<::mlir::lir::RotYOp>(odsLoc, QubitType::get(op0->getContext()), castedOp0.qin(), castedOp0.cin());

        rewriter.replaceOp(op0, tblgen_RotYOp_0.getResult());
        return ::mlir::success();
    };
};

struct DoubleXPattern : public ::mlir::RewritePattern
{
    DoubleXPattern(::mlir::MLIRContext *context)
        : ::mlir::RewritePattern("lir.rot_x", 2, context, {"lir.add_cvalue_q", "lir.rot_x"}) {}
    ::mlir::LogicalResult matchAndRewrite(::mlir::Operation *op0,
                                          ::mlir::PatternRewriter &rewriter) const override
    {
        // Variables for capturing values and attributes used while creating ops
        ::mlir::Operation::operand_range c1(op0->getOperands());
        ::mlir::Operation::operand_range c2(op0->getOperands());
        ::mlir::Operation::operand_range q(op0->getOperands());
        ::llvm::SmallVector<::mlir::Operation *, 4> tblgen_ops;

        // Match
        tblgen_ops.push_back(op0);
        auto castedOp0 = ::llvm::dyn_cast<::mlir::lir::RotXOp>(op0);
        (void)castedOp0;
        auto *op1 = (*castedOp0.getODSOperands(0).begin()).getDefiningOp();
        {
            if (!(op1))
            {
                return rewriter.notifyMatchFailure(castedOp0, [&](::mlir::Diagnostic &diag)
                                                   { diag << "Operand 0 of castedOp0 has null definingOp"; });
            }
            auto castedOp1 = ::llvm::dyn_cast<::mlir::lir::RotXOp>(op1);
            (void)castedOp1;
            if (!(castedOp1))
            {
                return rewriter.notifyMatchFailure(op1, [&](::mlir::Diagnostic &diag)
                                                   { diag << "castedOp1 is not ::mlir::lir::RotXOp type"; });
            }
            q = castedOp1.getODSOperands(0);
            c1 = castedOp1.getODSOperands(1);
            tblgen_ops.push_back(op1);
        }
        c2 = castedOp0.getODSOperands(1);

        // Rewrite
        auto odsLoc = rewriter.getFusedLoc({tblgen_ops[0]->getLoc(), tblgen_ops[1]->getLoc()});
        (void)odsLoc;
        ::llvm::SmallVector<::mlir::Value, 4> tblgen_repl_values;
        ::mlir::lir::AddCValueQOp tblgen_AddCValueQOp_0;
        {
            ::mlir::Value tblgen_value_0 = (*c1.begin());
            ::mlir::Value tblgen_value_1 = (*c2.begin());
            tblgen_AddCValueQOp_0 = rewriter.create<::mlir::lir::AddCValueQOp>(odsLoc,
                                                                               /*cin0=*/tblgen_value_0,
                                                                               /*cin1=*/tblgen_value_1);
        }
        ::mlir::lir::RotXOp tblgen_RotXOp_1;
        {
            ::mlir::SmallVector<::mlir::Value, 4> tblgen_values;
            (void)tblgen_values;
            ::mlir::SmallVector<::mlir::NamedAttribute, 4> tblgen_attrs;
            (void)tblgen_attrs;
            tblgen_values.push_back((*q.begin()));
            tblgen_values.push_back((*tblgen_AddCValueQOp_0.getODSResults(0).begin()));
            ::mlir::SmallVector<::mlir::Type, 4> tblgen_types;
            (void)tblgen_types;
            for (auto v : castedOp0.getODSResults(0))
            {
                tblgen_types.push_back(v.getType());
            }
            tblgen_RotXOp_1 = rewriter.create<::mlir::lir::RotXOp>(odsLoc, tblgen_types, tblgen_values, tblgen_attrs);
        }

        for (auto v : ::llvm::SmallVector<::mlir::Value, 4>{tblgen_RotXOp_1.getODSResults(0)})
        {
            tblgen_repl_values.push_back(v);
        }

        rewriter.replaceOp(op0, tblgen_repl_values);
        rewriter.eraseOp(op1);
        return ::mlir::success();
    };
};

struct DoubleHadamardPattern : public ::mlir::RewritePattern
{
    DoubleHadamardPattern(::mlir::MLIRContext *context)
        : ::mlir::RewritePattern("lir.gate_h", 2, context, {}) {}
    ::mlir::LogicalResult matchAndRewrite(::mlir::Operation *op0,
                                          ::mlir::PatternRewriter &rewriter) const override
    {
        // Variables for capturing values and attributes used while creating ops
        ::mlir::Operation::operand_range q(op0->getOperands());
        ::llvm::SmallVector<::mlir::Operation *, 4> tblgen_ops;

        // Match
        tblgen_ops.push_back(op0);
        auto castedOp0 = ::llvm::dyn_cast<::mlir::lir::GateHOp>(op0);
        (void)castedOp0;
        auto *op1 = (*castedOp0.getODSOperands(0).begin()).getDefiningOp();
        {
            if (!(op1))
            {
                return rewriter.notifyMatchFailure(castedOp0, [&](::mlir::Diagnostic &diag)
                                                   { diag << "Operand 0 of castedOp0 has null definingOp"; });
            }
            auto castedOp1 = ::llvm::dyn_cast<::mlir::lir::GateHOp>(op1);
            (void)castedOp1;
            if (!(castedOp1))
            {
                return rewriter.notifyMatchFailure(op1, [&](::mlir::Diagnostic &diag)
                                                   { diag << "castedOp1 is not ::mlir::lir::GateHOp type"; });
            }
            q = castedOp1.getODSOperands(0);
            tblgen_ops.push_back(op1);
        }

        // Rewrite
        auto odsLoc = rewriter.getFusedLoc({tblgen_ops[0]->getLoc(), tblgen_ops[1]->getLoc()});
        (void)odsLoc;
        ::llvm::SmallVector<::mlir::Value, 4> tblgen_repl_values;

        for (auto v : ::llvm::SmallVector<::mlir::Value, 4>{q})
        {
            tblgen_repl_values.push_back(v);
        }

        rewriter.replaceOp(op0, tblgen_repl_values);
        rewriter.eraseOp(op1);
        return ::mlir::success();
    };
};

void LirRewritePass::runOnFunction()
{
    OwningRewritePatternList patterns(&getContext());
    populateWithGenerated(patterns);

    patterns.add<
        // SendReorderPattern,
        AllocReorderPattern,
        EntangleReorderPattern,
        DoubleXPattern,
        DoubleHadamardPattern,
        GateCphaseReorderPattern>(&getContext());
    // patterns.add<RotXPattern>(&getContext());
    // patterns.add<ThisOneWorks>(&getContext());

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

    std::unique_ptr<FunctionPass> createLirRewritePass()
    {
        return std::make_unique<LirRewritePass>();
    }

} // namespace mlir
