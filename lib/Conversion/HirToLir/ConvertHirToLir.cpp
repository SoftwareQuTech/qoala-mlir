#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/SCF/SCF.h"
#include "mlir/Dialect/StandardOps/IR/Ops.h"
#include "mlir/Transforms/DialectConversion.h"

#include "Conversion/HirToLir/Passes.h"
#include "Dialect/lir/LirDialect.h"
#include "Dialect/lir/LirOps.h"
#include "Dialect/hir/HirDialect.h"
#include "Dialect/hir/HirOps.h"
#include "PassDetail.h"

using namespace mlir;

namespace
{

  // Maps qssa.qubit to the corresponding qasm.qubit
  // For now only supports static size
  class QubitMap
  {
  public:
    QubitMap(MLIRContext *ctx) : ctx(ctx), qubits() {}
    void allocate(Value arg, Value q)
    {
      qubits[arg] = q;
    }
    // void concat(Value lhs, Value rhs, Value res)
    // {
    //   qubits[res] = qubits[lhs];
    //   qubits[res].insert(qubits[res].end(), qubits[rhs].begin(),
    //                      qubits[rhs].end());
    // }
    // void split(Value arg, Value resLhs, Value resRhs)
    // {
    //   int lhsSize = resLhs.getType().cast<hir::QubitType>().getSize();
    //   qubits[resLhs] = {qubits[arg].begin(), qubits[arg].begin() + lhsSize};
    //   qubits[resRhs] = {qubits[arg].begin() + lhsSize, qubits[arg].end()};
    // };
    Value resolve(Value arg) { return arg; }

    MLIRContext *getContext() const { return ctx; }

  private:
    [[maybe_unused]] MLIRContext *ctx;
    llvm::DenseMap<Value, Value> qubits;
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
                    { return lir::QubitType(); });
    }
    MLIRContext *getContext() const { return context; }

  private:
    MLIRContext *context;
  };

  template <typename Op>
  struct HirToLirOpConversion : OpConversionPattern<Op>
  {
    QubitMap *qubitMap;
    Type getLirQubitType() const
    {
      return lir::QubitType::get(qubitMap->getContext());
    }

    HirToLirOpConversion(MLIRContext *ctx, QubitMap *qubitMap)
        : OpConversionPattern<Op>(ctx), qubitMap(qubitMap) {}
  };

  struct AllocateOpConversion : HirToLirOpConversion<hir::AllocateOp>
  {
    using HirToLirOpConversion::HirToLirOpConversion;
    LogicalResult
    matchAndRewrite(hir::AllocateOp op, ArrayRef<Value> operands,
                    ConversionPatternRewriter &rewriter) const final
    {
      auto qubit =
          rewriter.create<lir::AllocateOp>(op->getLoc(), getLirQubitType());
      qubitMap->allocate(op.getResult(), qubit);
      rewriter.eraseOp(op);
      return success();
    }
  };

  void populateHirToLirConversionPatterns(
      HirTypeConverter &typeConverter, QubitMap &qubitMap,
      OwningRewritePatternList &patterns)
  {
    // clang-format off
  patterns.insert<
      AllocateOpConversion
  >(patterns.getContext(), &qubitMap);
    // clang-format on
  }

  struct HirToLirTarget : public ConversionTarget
  {
    HirToLirTarget(MLIRContext &ctx) : ConversionTarget(ctx)
    {
      addLegalDialect<StandardOpsDialect>();
      addLegalDialect<lir::LirDialect>();
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
    QubitMap qubitMap(&getContext());
    populateHirToLirConversionPatterns(typeConverter, qubitMap, patterns);

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
