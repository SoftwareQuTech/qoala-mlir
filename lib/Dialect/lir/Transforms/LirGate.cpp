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

class LirGatePass : public LirGatePassBase<LirGatePass>
{
    void runOnFunction() override;
};

namespace
{
#include "Dialect/lir/Transforms/LirRewrites.h.inc"
} // namespace

template <typename TyOp>
struct LirGatePattern : public OpRewritePattern<TyOp>
{
public:
    using OpRewritePattern<TyOp>::OpRewritePattern;
};

struct RotXPattern : public LirGatePattern<RotXOp>
{
public:
    using LirGatePattern<RotXOp>::LirGatePattern;

    LogicalResult matchAndRewrite(RotXOp op, PatternRewriter &rewriter) const override
    {
        return failure();
    }
};

struct DoubleXPattern : public ::mlir::RewritePattern
{
    DoubleXPattern(::mlir::MLIRContext *context)
        : ::mlir::RewritePattern("lir.rot_x", 2, context, {"lir.add_cval_q", "lir.rot_x"}) {}
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

struct RotZBeforeMeasPattern : public ::mlir::RewritePattern
{
    RotZBeforeMeasPattern(::mlir::MLIRContext *context)
        : ::mlir::RewritePattern("lir.meas", 2, context, {"lir.meas"}) {}
    ::mlir::LogicalResult matchAndRewrite(::mlir::Operation *op0,
                                          ::mlir::PatternRewriter &rewriter) const override
    {
        // Variables for capturing values and attributes used while creating ops
        ::mlir::Operation::operand_range q(op0->getOperands());
        ::llvm::SmallVector<::mlir::Operation *, 4> tblgen_ops;

        // Match
        tblgen_ops.push_back(op0);
        auto castedOp0 = ::llvm::dyn_cast<::mlir::lir::MeasOp>(op0);
        (void)castedOp0;
        auto *op1 = (*castedOp0.getODSOperands(0).begin()).getDefiningOp();
        {
            if (!(op1))
            {
                return rewriter.notifyMatchFailure(castedOp0, [&](::mlir::Diagnostic &diag)
                                                   { diag << "Operand 0 of castedOp0 has null definingOp"; });
            }
            auto castedOp1 = ::llvm::dyn_cast<::mlir::lir::RotZOp>(op1);
            (void)castedOp1;
            if (!(castedOp1))
            {
                return rewriter.notifyMatchFailure(op1, [&](::mlir::Diagnostic &diag)
                                                   { diag << "castedOp1 is not ::mlir::lir::RotZOp type"; });
            }
            q = castedOp1.getODSOperands(0);
            tblgen_ops.push_back(op1);
        }

        // Rewrite
        auto odsLoc = rewriter.getFusedLoc({tblgen_ops[0]->getLoc(), tblgen_ops[1]->getLoc()});
        (void)odsLoc;
        ::llvm::SmallVector<::mlir::Value, 4> tblgen_repl_values;
        ::mlir::lir::MeasOp tblgen_MeasOp_0;
        {
            ::mlir::SmallVector<::mlir::Value, 4> tblgen_values;
            (void)tblgen_values;
            ::mlir::SmallVector<::mlir::NamedAttribute, 4> tblgen_attrs;
            (void)tblgen_attrs;
            tblgen_values.push_back((*q.begin()));
            ::mlir::SmallVector<::mlir::Type, 4> tblgen_types;
            (void)tblgen_types;
            for (auto v : castedOp0.getODSResults(0))
            {
                tblgen_types.push_back(v.getType());
            }
            tblgen_MeasOp_0 = rewriter.create<::mlir::lir::MeasOp>(odsLoc, tblgen_types, tblgen_values, tblgen_attrs);
        }

        for (auto v : ::llvm::SmallVector<::mlir::Value, 4>{tblgen_MeasOp_0.getODSResults(0)})
        {
            tblgen_repl_values.push_back(v);
        }

        rewriter.replaceOp(op0, tblgen_repl_values);
        rewriter.eraseOp(op1);
        return ::mlir::success();
    };
};

void LirGatePass::runOnFunction()
{
    OwningRewritePatternList patterns(&getContext());
    populateWithGenerated(patterns);

    patterns.add<
        DoubleXPattern,
        DoubleHadamardPattern,
        RotZBeforeMeasPattern
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

    std::unique_ptr<FunctionPass> createLirGatePass()
    {
        return std::make_unique<LirGatePass>();
    }

} // namespace mlir
