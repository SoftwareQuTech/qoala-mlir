#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/SCF/SCF.h"
#include "mlir/Dialect/StandardOps/IR/Ops.h"
#include "mlir/Transforms/DialectConversion.h"

#include "Conversion/HirToLir/Passes.h"
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
          rewriter.create<lir::AllocateOp>(op->getLoc(), getLirQubitType());
      valueMap->allocate_qubit(op.getResult(), qubit);
      rewriter.eraseOp(op);
      return success();
    }
  };

  struct EntangleOpConversion : HirToLirOpConversion<hir::EntangleOp>
  {
    using HirToLirOpConversion::HirToLirOpConversion;
    LogicalResult
    matchAndRewrite(hir::EntangleOp op, ArrayRef<Value> operands,
                    ConversionPatternRewriter &rewriter) const final
    {
      auto qubit =
          rewriter.create<lir::EntangleOp>(op->getLoc(), getLirQubitType());
      valueMap->allocate_qubit(op.getResult(), qubit);
      rewriter.eraseOp(op);
      return success();
    }
  };

  struct NewCValueOpConversion : HirToLirOpConversion<hir::NewCValueOp>
  {
    using HirToLirOpConversion::HirToLirOpConversion;
    LogicalResult
    matchAndRewrite(hir::NewCValueOp op, ArrayRef<Value> operands,
                    ConversionPatternRewriter &rewriter) const final
    {
      auto new_op =
          rewriter.create<lir::NewCValueCOp>(op->getLoc(), getLirCValueCType());
      valueMap->allocate_cvalue_on_c(op.getResult(), new_op);
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
          rewriter.create<lir::RecvCMsgOp>(op->getLoc(), getLirCValueCType());
      valueMap->allocate_cvalue_on_c(op.getResult(), new_op);
      rewriter.eraseOp(op);
      return success();
    }
  };

  struct SendCMsgOpConversion : HirToLirOpConversion<hir::SendCMsgOp>
  {
    using HirToLirOpConversion::HirToLirOpConversion;
    LogicalResult
    matchAndRewrite(hir::SendCMsgOp op, ArrayRef<Value> operands,
                    ConversionPatternRewriter &rewriter) const final
    {
      hir::SendCMsgOpAdaptor args(operands);

      // check if value is already on C side
      Value cvalue_on_c;
      if (valueMap->has_cvalue_on_c_values(args.cin()))
      {
        cvalue_on_c = valueMap->resolve_cvalue_on_c(args.cin());
      }
      else
      {
        assert(valueMap->has_cvalue_on_q_values(args.cin()));
        auto value_lir = valueMap->resolve_cvalue_on_q(args.cin());
        cvalue_on_c = rewriter.create<lir::CValueQtoCOp>(op->getLoc(), getLirCValueCType(), value_lir);
        valueMap->allocate_cvalue_on_c(args.cin(), cvalue_on_c);
      }

      rewriter.create<lir::SendCMsgOp>(op->getLoc(), cvalue_on_c);
      rewriter.eraseOp(op);
      return success();
    }
  };

  class FuncOpConversion : public HirToLirOpConversion<FuncOp>
  {
  public:
    using HirToLirOpConversion::HirToLirOpConversion;
    LogicalResult
    matchAndRewrite(FuncOp funcOp, ArrayRef<Value> operands,
                    ConversionPatternRewriter &rewriter) const override
    {
      // llvm::outs() << "rewriting func op\n";

      // auto funcType =
      //     FunctionType::get(funcOp->getContext(), mlir::NoneType::get(funcOp->getContext()),
      //                       lir::QubitType::get(funcOp->getContext()));

      // auto newFuncOp =
      //     rewriter.create<FuncOp>(funcOp->getLoc(), funcOp.getName(), funcType);

      // rewriter.inlineRegionBefore(funcOp.getBody(), newFuncOp.getBody(),
      //                             newFuncOp.end());

      // rewriter.eraseOp(funcOp);

      // populate the qubit map
      return success();
    }
  };

  struct ReturnOpConversion : HirToLirOpConversion<ReturnOp>
  {
    using HirToLirOpConversion::HirToLirOpConversion;
    LogicalResult
    matchAndRewrite(ReturnOp op, ArrayRef<Value> operands,
                    ConversionPatternRewriter &rewriter) const final
    {
      // llvm::outs() << "rewriting ReturnOp\n";
      FuncOp func = op->getParentOfType<FuncOp>();
      auto q = func->getResult(0);
      // llvm::outs() << "this is q: " << q;
      auto qubit_lir = valueMap->resolve(q);
      // llvm::outs() << "this is qubit_lir: " << qubit_lir;
      rewriter.create<ReturnOp>(op->getLoc(), qubit_lir);
      rewriter.eraseOp(op);
      return success();
    }
  };

  struct RotXOpConversion : HirToLirOpConversion<hir::RotXOp>
  {
    using HirToLirOpConversion::HirToLirOpConversion;
    LogicalResult
    matchAndRewrite(hir::RotXOp op, ArrayRef<Value> operands,
                    ConversionPatternRewriter &rewriter) const final
    {
      // llvm::outs() << "rewriting RotXOp\n";

      lir::RotXOpAdaptor args(operands);

      // check if value is already on Q side
      Value cvalue_on_q;
      if (valueMap->has_cvalue_on_q_values(args.cin()))
      {
        cvalue_on_q = valueMap->resolve_cvalue_on_q(args.cin());
      }
      else
      {
        assert(valueMap->has_cvalue_on_c_values(args.cin()));
        auto value_lir = valueMap->resolve_cvalue_on_c(args.cin());
        cvalue_on_q = rewriter.create<lir::CValueCtoQOp>(op->getLoc(), getLirCValueQType(), value_lir);
        valueMap->allocate_cvalue_on_q(args.cin(), cvalue_on_q);
      }

      auto qubit_lir = valueMap->resolve(args.qin());
      auto qubit =
          rewriter.create<lir::RotXOp>(op->getLoc(), getLirQubitType(), qubit_lir, cvalue_on_q);
      valueMap->allocate_qubit(op.getResult(), qubit);
      rewriter.eraseOp(op);
      return success();
    }
  };

  struct RotYOpConversion : HirToLirOpConversion<hir::RotYOp>
  {
    using HirToLirOpConversion::HirToLirOpConversion;
    LogicalResult
    matchAndRewrite(hir::RotYOp op, ArrayRef<Value> operands,
                    ConversionPatternRewriter &rewriter) const final
    {
      lir::RotYOpAdaptor args(operands);

      // check if value is already on Q side
      Value cvalue_on_q;
      if (valueMap->has_cvalue_on_q_values(args.cin()))
      {
        cvalue_on_q = valueMap->resolve_cvalue_on_q(args.cin());
      }
      else
      {
        assert(valueMap->has_cvalue_on_c_values(args.cin()));
        auto value_lir = valueMap->resolve_cvalue_on_c(args.cin());
        cvalue_on_q = rewriter.create<lir::CValueCtoQOp>(op->getLoc(), getLirCValueQType(), value_lir);
        valueMap->allocate_cvalue_on_q(args.cin(), cvalue_on_q);
      }

      auto qubit_lir = valueMap->resolve(args.qin());
      auto qubit =
          rewriter.create<lir::RotYOp>(op->getLoc(), getLirQubitType(), qubit_lir, cvalue_on_q);
      valueMap->allocate_qubit(op.getResult(), qubit);
      rewriter.eraseOp(op);

      return success();
    }
  };

  struct RotZOpConversion : HirToLirOpConversion<hir::RotZOp>
  {
    using HirToLirOpConversion::HirToLirOpConversion;
    LogicalResult
    matchAndRewrite(hir::RotZOp op, ArrayRef<Value> operands,
                    ConversionPatternRewriter &rewriter) const final
    {
      // llvm::outs() << "rewriting RotZOp\n";

      lir::RotZOpAdaptor args(operands);

      // check if value is already on Q side
      Value cvalue_on_q;
      if (valueMap->has_cvalue_on_q_values(args.cin()))
      {
        cvalue_on_q = valueMap->resolve_cvalue_on_q(args.cin());
      }
      else
      {
        assert(valueMap->has_cvalue_on_c_values(args.cin()));
        auto value_lir = valueMap->resolve_cvalue_on_c(args.cin());
        cvalue_on_q = rewriter.create<lir::CValueCtoQOp>(op->getLoc(), getLirCValueQType(), value_lir);
        valueMap->allocate_cvalue_on_q(args.cin(), cvalue_on_q);
      }

      auto qubit_lir = valueMap->resolve(args.qin());
      auto qubit =
          rewriter.create<lir::RotZOp>(op->getLoc(), getLirQubitType(), qubit_lir, cvalue_on_q);
      valueMap->allocate_qubit(op.getResult(), qubit);
      rewriter.eraseOp(op);
      return success();
    }
  };

  struct GateHOpConversion : HirToLirOpConversion<hir::GateHOp>
  {
    using HirToLirOpConversion::HirToLirOpConversion;
    LogicalResult
    matchAndRewrite(hir::GateHOp op, ArrayRef<Value> operands,
                    ConversionPatternRewriter &rewriter) const final
    {
      // llvm::outs() << "rewriting RotZOp\n";

      lir::GateHOpAdaptor args(operands);

      auto qubit_lir = valueMap->resolve(args.qin());
      auto qubit =
          rewriter.create<lir::GateHOp>(op->getLoc(), getLirQubitType(), qubit_lir);
      valueMap->allocate_qubit(op.getResult(), qubit);
      rewriter.eraseOp(op);
      return success();
    }
  };

  struct GateCphaseOpConversion : HirToLirOpConversion<hir::GateCphaseOp>
  {
    using HirToLirOpConversion::HirToLirOpConversion;
    LogicalResult
    matchAndRewrite(hir::GateCphaseOp op, ArrayRef<Value> operands,
                    ConversionPatternRewriter &rewriter) const final
    {
      lir::GateCphaseOpAdaptor args(operands);

      auto qubit_lir0 = valueMap->resolve(args.qin0());
      auto qubit_lir1 = valueMap->resolve(args.qin1());
      auto newOp =
          rewriter.create<lir::GateCphaseOp>(
              op->getLoc(), getLirQubitType(), getLirQubitType(), qubit_lir0, qubit_lir1);
      valueMap->allocate_qubit(op.getResult(0), newOp.getResult(0));
      valueMap->allocate_qubit(op.getResult(1), newOp.getResult(1));
      rewriter.eraseOp(op);
      return success();
    }
  };

  struct GateCnotOpConversion : HirToLirOpConversion<hir::GateCnotOp>
  {
    using HirToLirOpConversion::HirToLirOpConversion;
    LogicalResult
    matchAndRewrite(hir::GateCnotOp op, ArrayRef<Value> operands,
                    ConversionPatternRewriter &rewriter) const final
    {
      lir::GateCnotOpAdaptor args(operands);

      auto qubit_lir0 = valueMap->resolve(args.qin0());
      auto qubit_lir1 = valueMap->resolve(args.qin1());
      auto newOp =
          rewriter.create<lir::GateCnotOp>(
              op->getLoc(), getLirQubitType(), getLirQubitType(), qubit_lir0, qubit_lir1);
      valueMap->allocate_qubit(op.getResult(0), newOp.getResult(0));
      valueMap->allocate_qubit(op.getResult(1), newOp.getResult(1));
      rewriter.eraseOp(op);
      return success();
    }
  };

  struct MeasOpConversion : HirToLirOpConversion<hir::MeasOp>
  {
    using HirToLirOpConversion::HirToLirOpConversion;
    LogicalResult
    matchAndRewrite(hir::MeasOp op, ArrayRef<Value> operands,
                    ConversionPatternRewriter &rewriter) const final
    {
      // llvm::outs() << "rewriting MeasOp\n";

      lir::MeasOpAdaptor args(operands);

      auto qubit_lir = valueMap->resolve(args.qin());
      auto outcome =
          rewriter.create<lir::MeasOp>(op->getLoc(), getLirCValueQType(), qubit_lir);
      valueMap->allocate_cvalue_on_q(op.getResult(), outcome);
      rewriter.eraseOp(op);
      return success();
    }
  };

  // struct AddCValuesConversion : HirToLirOpConversion<hir::AddCValuesOp>
  // {
  //   using HirToLirOpConversion::HirToLirOpConversion;
  //   LogicalResult
  //   matchAndRewrite(hir::AddCValuesOp op, ArrayRef<Value> operands,
  //                   ConversionPatternRewriter &rewriter) const final
  //   {
  //     // llvm::outs() << "rewriting RotXOp\n";

  //     lir::AddCValuesOpAdaptor args(operands);

  //     // check if value is already on Q side
  //     Value cvalue_on_q;
  //     if (valueMap->has_cvalue_on_q_values(args.cin()))
  //     {
  //       cvalue_on_q = valueMap->resolve_cvalue_on_q(args.cin());
  //     }
  //     else
  //     {
  //       assert(valueMap->has_cvalue_on_c_values(args.cin()));
  //       auto value_lir = valueMap->resolve_cvalue_on_c(args.cin());
  //       cvalue_on_q = rewriter.create<lir::CValueCtoQOp>(op->getLoc(), getLirCValueQType(), value_lir);
  //       valueMap->allocate_cvalue_on_q(args.cin(), cvalue_on_q);
  //     }

  //     auto qubit_lir = valueMap->resolve(args.qin());
  //     auto qubit =
  //         rewriter.create<lir::RotXOp>(op->getLoc(), getLirQubitType(), qubit_lir, cvalue_on_q);
  //     valueMap->allocate_qubit(op.getResult(), qubit);
  //     rewriter.eraseOp(op);
  //     return success();
  //   }
  // };

  void populateHirToLirConversionPatterns(
      HirTypeConverter &typeConverter, ValueMap &valueMap,
      OwningRewritePatternList &patterns)
  {
    // clang-format off
  patterns.insert<
      AllocateOpConversion,
      EntangleOpConversion,
      SendCMsgOpConversion,
      RecvCMsgOpConversion,
      RotXOpConversion,
      RotYOpConversion,
      RotZOpConversion,
      GateHOpConversion,
      GateCphaseOpConversion,
      GateCnotOpConversion,
      NewCValueOpConversion,
      // FuncOpConversion,
      ReturnOpConversion,
      MeasOpConversion
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
      addLegalDialect<lir::LirDialect>();
      addLegalDialect<AffineDialect>();

      // addIllegalDialect<hir::HirDialect>();
    }
  };

  struct HirToLirPass : public HirToLirPassBase<HirToLirPass>
  {
    void runOnOperation() override;
  };

  void printOperation(Operation *op);

  void printBlock(Block &block)
  {
    // Print the block intrinsics properties (basically: argument list)
    llvm::outs()
        << "Block with " << block.getNumArguments() << " arguments, "
        << block.getNumSuccessors()
        << " successors, and "
        // Note, this `.size()` is traversing a linked-list and is O(n).
        << block.getOperations().size() << " operations\n";

    // A block main role is to hold a list of Operations: let's recurse into
    // printing each operation.
    for (Operation &op : block.getOperations())
      printOperation(&op);
  }

  void printRegion(Region &region)
  {
    // A region does not hold anything by itself other than a list of blocks.
    llvm::outs() << "Region with " << region.getBlocks().size()
                 << " blocks:\n";
    for (Block &block : region.getBlocks())
      printBlock(block);
  }

  void printOperation(Operation *op)
  {
    // Print the operation itself and some of its properties
    llvm::outs() << "visiting op: '" << op->getName() << "' with "
                 << op->getNumOperands() << " operands and "
                 << op->getNumResults() << " results\n";
    // Print the operation attributes
    if (!op->getAttrs().empty())
    {
      llvm::outs() << op->getAttrs().size() << " attributes:\n";
      for (NamedAttribute attr : op->getAttrs())
        llvm::outs() << " - '" << attr.first << "' : '" << attr.second
                     << "'\n";
    }

    // Recurse into each of the regions attached to the operation.
    llvm::outs() << " " << op->getNumRegions() << " nested regions:\n";
    for (Region &region : op->getRegions())
      printRegion(region);
  }

  void HirToLirPass::runOnOperation()
  {
    OwningRewritePatternList patterns(&getContext());
    HirTypeConverter typeConverter(&getContext());
    ValueMap valueMap(&getContext());
    populateHirToLirConversionPatterns(typeConverter, valueMap, patterns);

    HirToLirTarget target(getContext());

    Operation *op = getOperation();
    // printOperation(op);

    // llvm::outs() << "\n converting operation " << *op << "\n";

    if (failed(applyPartialConversion(op, target,
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
