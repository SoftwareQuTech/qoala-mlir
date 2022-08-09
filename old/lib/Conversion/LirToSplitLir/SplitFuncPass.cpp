#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/SCF/SCF.h"
#include "mlir/Dialect/StandardOps/IR/Ops.h"
#include "mlir/Transforms/DialectConversion.h"

#include "Conversion/LirToSplitLir/Passes.h"
#include "Dialect/clir/CLirDialect.h"
#include "Dialect/clir/CLirOps.h"
#include "Dialect/qlir/QLirDialect.h"
#include "Dialect/qlir/QLirOps.h"
#include "Dialect/lir/LirDialect.h"
#include "Dialect/lir/LirOps.h"
#include "Dialect/hir/HirDialect.h"
#include "Dialect/hir/HirOps.h"
#include "PassDetail.h"

using namespace mlir;

namespace
{

  class ValueMap
  {
  public:
    ValueMap(MLIRContext *ctx) : ctx(ctx), qubits(), cvalues_on_c(), cvalues_on_q() {}
    void allocate_qubit(Value arg, Value q)
    {
      qubits[arg] = q;
    }

    void allocate_cvalue_on_c(Value arg, Value c)
    {
      cvalues_on_c[arg] = c;
    }
    void allocate_cvalue_on_q(Value arg, Value c)
    {
      cvalues_on_q[arg] = c;
    }
    Value resolve(Value arg) { return qubits[arg]; }
    Value resolve_cvalue_on_c(Value arg) { return cvalues_on_c[arg]; }
    Value resolve_cvalue_on_q(Value arg) { return cvalues_on_q[arg]; }

    bool has_cvalue_on_c_values(Value arg)
    {
      return cvalues_on_c.find(arg) != cvalues_on_c.end();
    }
    bool has_cvalue_on_q_values(Value arg)
    {
      return cvalues_on_q.find(arg) != cvalues_on_q.end();
    }

    MLIRContext *getContext() const { return ctx; }

  private:
    MLIRContext *ctx;
    // maps from values in old dialect to values in new dialect
    llvm::DenseMap<Value, Value> qubits;
    llvm::DenseMap<Value, Value> cvalues_on_c;
    llvm::DenseMap<Value, Value> cvalues_on_q;
  };

  template <typename Op>
  struct SplitFuncOpConversion : OpConversionPattern<Op>
  {
    ValueMap *valueMap;
    Type getLirQubitType() const
    {
      return lir::QubitType::get(valueMap->getContext());
    }

    Type getLirCValueCType() const
    {
      return lir::CValueCType::get(valueMap->getContext());
    }
    Type getLirCValueQType() const
    {
      return lir::CValueQType::get(valueMap->getContext());
    }

    SplitFuncOpConversion(MLIRContext *ctx, ValueMap *valueMap)
        : OpConversionPattern<Op>(ctx), valueMap(valueMap) {}
  };

  class FuncOpConversion : public SplitFuncOpConversion<FuncOp>
  {
  public:
    using SplitFuncOpConversion::SplitFuncOpConversion;
    LogicalResult
    matchAndRewrite(FuncOp funcOp, ArrayRef<Value> operands,
                    ConversionPatternRewriter &rewriter) const override
    {
      llvm::outs() << "rewriting func op\n";

      auto funcType =
          FunctionType::get(funcOp->getContext(), lir::QubitType::get(funcOp->getContext()),
                            lir::QubitType::get(funcOp->getContext()));

      // auto new_name = funcOp.getName() + "2";
      auto newFuncOp =
          rewriter.create<FuncOp>(funcOp->getLoc(), "newfunc", funcType);
      newFuncOp.setPrivate();

      rewriter.inlineRegionBefore(funcOp.getBody(), newFuncOp.getBody(),
                                  newFuncOp.end());

      rewriter.eraseOp(funcOp);

      // populate the qubit map
      return success();
    }
  };

  void populateLirToSplitLirConversionPatterns(
      ValueMap &valueMap,
      OwningRewritePatternList &patterns)
  {
    // clang-format off
  patterns.insert<
      FuncOpConversion
  >(patterns.getContext(), &valueMap);
    // clang-format on
  }

  struct SplitFuncTarget : public ConversionTarget
  {
    SplitFuncTarget(MLIRContext &ctx) : ConversionTarget(ctx)
    {
      addLegalDialect<StandardOpsDialect>();
      addLegalDialect<qlir::QLirDialect>();
      addLegalDialect<clir::CLirDialect>();
      addLegalDialect<lir::LirDialect>();
    }
  };

  struct SplitFuncPass : public SplitFuncPassBase<SplitFuncPass>
  {
    void runOnOperation() override;
  };

  void SplitFuncPass::runOnOperation()
  {
    OwningRewritePatternList patterns(&getContext());
    ValueMap valueMap(&getContext());
    populateLirToSplitLirConversionPatterns(valueMap, patterns);

    SplitFuncTarget target(getContext());

    Operation *op = getOperation();
    llvm::outs() << "\n converting operation " << *op << "\n";

    if (failed(applyPartialConversion(op, target,
                                      std::move(patterns))))
    {
      return signalPassFailure();
    }
  }
} // namespace

namespace mlir
{
  std::unique_ptr<Pass> createSplitFuncPass()
  {
    return std::make_unique<SplitFuncPass>();
  }
} // namespace mlir
