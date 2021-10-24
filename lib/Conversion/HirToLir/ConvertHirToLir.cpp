#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/SCF/SCF.h"
#include "mlir/Dialect/StandardOps/IR/Ops.h"
#include "mlir/Transforms/DialectConversion.h"

#include "Conversion/HirToLir/Passes.h"
#include "Dialect/clir/CLirDialect.h"
#include "Dialect/clir/CLirOps.h"
#include "Dialect/qlir/QLirDialect.h"
#include "Dialect/qlir/QLirOps.h"
#include "Dialect/lircommon/LirCommonDialect.h"
#include "Dialect/lircommon/LirCommonOps.h"
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

  class HirTypeConverter : public TypeConverter
  {
  public:
    using TypeConverter::convertType;

    HirTypeConverter(MLIRContext *context) : context(context)
    {
      addConversion([](Type type)
                    { return type; });
      addConversion([&](hir::QubitType type)
                    { return lircommon::QubitType(); });
    }
    MLIRContext *getContext() const { return context; }

  private:
    MLIRContext *context;
  };

  template <typename Op>
  struct HirToLirOpConversion : OpConversionPattern<Op>
  {
    ValueMap *valueMap;
    Type getLirQubitType() const
    {
      return lircommon::QubitType::get(valueMap->getContext());
    }

    Type getLirCValueCType() const
    {
      return lircommon::CValueOnClasType::get(valueMap->getContext());
    }
    Type getLirCValueQType() const
    {
      return lircommon::CValueOnQuanType::get(valueMap->getContext());
    }

    HirToLirOpConversion(MLIRContext *ctx, ValueMap *valueMap)
        : OpConversionPattern<Op>(ctx), valueMap(valueMap) {}
  };

  struct AllocateOpConversion : HirToLirOpConversion<hir::AllocateOp>
  {
    using HirToLirOpConversion::HirToLirOpConversion;
    LogicalResult
    matchAndRewrite(hir::AllocateOp op, ArrayRef<Value> operands,
                    ConversionPatternRewriter &rewriter) const final
    {
      auto qubit =
          rewriter.create<lircommon::AllocateOp>(op->getLoc(), getLirQubitType());
      valueMap->allocate_qubit(op.getResult(), qubit);
      rewriter.eraseOp(op);
      return success();
    }
  };

  struct RecvCMsgOpConversion : HirToLirOpConversion<hir::RecvCMsgOp>
  {
    using HirToLirOpConversion::HirToLirOpConversion;
    LogicalResult
    matchAndRewrite(hir::RecvCMsgOp op, ArrayRef<Value> operands,
                    ConversionPatternRewriter &rewriter) const final
    {
      auto new_op =
          rewriter.create<lircommon::RecvCMsgOp>(op->getLoc(), getLirCValueCType());
      valueMap->allocate_cvalue_on_c(op.getResult(), new_op);
      rewriter.eraseOp(op);
      return success();
    }
  };

  struct GateXOpConversion : HirToLirOpConversion<hir::GateXOp>
  {
    using HirToLirOpConversion::HirToLirOpConversion;
    LogicalResult
    matchAndRewrite(hir::GateXOp op, ArrayRef<Value> operands,
                    ConversionPatternRewriter &rewriter) const final
    {
      lircommon::GateXOpAdaptor args(operands);

      // check if value is already on Q side
      Value cvalue_on_q;
      if (valueMap->has_cvalue_on_q_values(args.cin()))
      {
        cvalue_on_q = valueMap->resolve_cvalue_on_q(args.cin());
      }
      else
      {
        assert(valueMap->has_cvalue_on_c_values(args.cin()));
        auto value_lircommon = valueMap->resolve_cvalue_on_c(args.cin());
        cvalue_on_q = rewriter.create<lircommon::CValueClassToQuanOp>(op->getLoc(), getLirCValueQType(), value_lircommon);
        valueMap->allocate_cvalue_on_q(args.cin(), cvalue_on_q);
      }

      auto qubit_lircommon = valueMap->resolve(args.qin());
      auto qubit =
          rewriter.create<lircommon::GateXOp>(op->getLoc(), getLirQubitType(), qubit_lircommon, cvalue_on_q);
      valueMap->allocate_qubit(op.getResult(), qubit);
      rewriter.eraseOp(op);
      return success();
    }
  };

  void populateHirToLirConversionPatterns(
      HirTypeConverter &typeConverter, ValueMap &valueMap,
      OwningRewritePatternList &patterns)
  {
    // clang-format off
  patterns.insert<
      AllocateOpConversion
  >(patterns.getContext(), &valueMap);

  patterns.insert<
      RecvCMsgOpConversion
  >(patterns.getContext(), &valueMap);

  patterns.insert<
      GateXOpConversion
  >(patterns.getContext(), &valueMap);
    // clang-format on
  }

  struct HirToLirTarget : public ConversionTarget
  {
    HirToLirTarget(MLIRContext &ctx) : ConversionTarget(ctx)
    {
      addLegalDialect<StandardOpsDialect>();
      addLegalDialect<qlir::QLirDialect>();
      addLegalDialect<clir::CLirDialect>();
      addLegalDialect<lircommon::LirCommonDialect>();
      addLegalDialect<AffineDialect>();

      addIllegalDialect<hir::HirDialect>();
    }
  };

  struct HirToLirPass : public HirToLirPassBase<HirToLirPass>
  {
    void runOnOperation() override;
  };

  void HirToLirPass::runOnOperation()
  {
    OwningRewritePatternList patterns(&getContext());
    HirTypeConverter typeConverter(&getContext());
    ValueMap valueMap(&getContext());
    populateHirToLirConversionPatterns(typeConverter, valueMap, patterns);

    HirToLirTarget target(getContext());
    if (failed(applyPartialConversion(getOperation(), target,
                                      std::move(patterns))))
    {
      return signalPassFailure();
    }
  }
} // namespace

namespace mlir
{
  std::unique_ptr<Pass> createHirToLirPass()
  {
    return std::make_unique<HirToLirPass>();
  }
} // namespace mlir
