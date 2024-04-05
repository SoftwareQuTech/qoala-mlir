#include "Conversion/QNetToQMem/QNetToQMemPatterns.h"
#include "mlir/Dialect/Tensor/IR/Tensor.h"
#include "mlir/Dialect/Arith/IR/Arith.h"

namespace qoala::conversion {
    /* Implementation of the qoala types converter */
    QNetToQMemQubitTypeConverter::QNetToQMemQubitTypeConverter(MLIRContext *ctx) {
        // Default conversion for non qnet::QubitType instances
        addConversion([](Type type) { return type; });
        addConversion([ctx](qnet::QubitType type) -> Type {
            // Qubit Types are mapped into i32 (pointers)
            return IntegerType::get(ctx, 32);
        });
        // In case there will be illegal values (values of types declared illegal)
        // after conversion, then we need to provide a "materialization", i.e. how
        // to cast from one type to the other Here we provide a "cast" to transform
        // from source type (qnet::QubitType) to target type (lir::QubitType)
        addTargetMaterialization([&](OpBuilder &builder, Type resultType,
                                     ValueRange inputs, Location loc) -> Value {
            // The conversion is simply a "builtin::unrealized_cast" operation
            return builder
                    .create<UnrealizedConversionCastOp>(loc, resultType, inputs)
                    .getResult(0);
        });
        // Here we provide a "cast" to transform from target type (lir::QubitType)
        // to source type (qnet::QubitType)
        addSourceMaterialization([&](OpBuilder &builder, Type resultType,
                                     ValueRange inputs,
                                     Location loc) -> std::optional<Value> {
            // The conversion is simply a "builtin::unrealized_cast" operation
            return builder
                    .create<UnrealizedConversionCastOp>(loc, resultType, inputs)
                    .getResult(0);
        });
    }

    /* Implementation of the lowering for the creation of new qubits */
    qmem::QAllocOp
    NewQubitLowering::createNewOp(qnet::NewQubitOp op, qnet::NewQubitOp::Adaptor adaptor,
                                  ConversionPatternRewriter &rewriter) const {
        Location loc = op.getLoc();
        // First, we convert the qnet.qubit type into its mapped type
        Type mappedQubitType = typeConverter->convertType(op.getQout().getType());

        // Second, we create a QAllocOp instance that allocates a single qubit
        auto newAllocOp = rewriter.create<qmem::QAllocOp>(loc, mappedQubitType);

        // Third, we introduce the "init" operation required by the allocation
        auto newInitOp = rewriter.create<qmem::InitOp>(loc, newAllocOp.getResult());
        return dyn_cast<qmem::QAllocOp>(newInitOp.getQ().getDefiningOp());
    }

    RotateXLowering::NewOpAndValues
    RotateXLowering::createNewOpAndValues(
            qnet::RotXOp op, qnet::RotXOp::Adaptor adaptor, ConversionPatternRewriter &rewriter) const {
        llvm::dbgs() << "lowering operation : '" << op << "'\n";
        // Since we move away from SSA, we need to replace all the uses of the output of the operation with
        // the mapped value of the "qin" operand of this operation
        rewriter.replaceAllUsesWith(op.getQout(), adaptor.getQin());
        auto newRotate = rewriter.create<qmem::RotateXOp>(op.getLoc(), adaptor.getQin(), adaptor.getAngle());
        // This is a tricky replacement.... we need to replace the operation *WITH THE VALUES OF THE OPERANDS*
        // which are the "modified" values on the qubits
        return RotateXLowering::NewOpAndValues(newRotate, ValueRange{newRotate.getQ()});
    }

    RotateYLowering::NewOpAndValues
    RotateYLowering::createNewOpAndValues(
            qnet::RotYOp op, qnet::RotYOp::Adaptor adaptor, ConversionPatternRewriter &rewriter) const {
        llvm::dbgs() << "lowering operation : '" << op << "'\n";
        // Since we move away from SSA, we need to replace all the uses of the output of the operation with
        // the mapped value of the "qin" operand of this operation
        rewriter.replaceAllUsesWith(op.getQout(), adaptor.getQin());
        auto newRotate = rewriter.create<qmem::RotateYOp>(op.getLoc(), adaptor.getQin(), adaptor.getAngle());
        // This is a tricky replacement.... we need to replace the operation *WITH THE VALUES OF THE OPERANDS*
        // which are the "modified" values on the qubits
        return RotateYLowering::NewOpAndValues(newRotate, ValueRange{newRotate.getQ()});
    }

    RotateZLowering::NewOpAndValues
    RotateZLowering::createNewOpAndValues(
            qnet::RotZOp op, qnet::RotZOp::Adaptor adaptor, ConversionPatternRewriter &rewriter) const {
        llvm::dbgs() << "lowering operation : '" << op << "'\n";
        // Since we move away from SSA, we need to replace all the uses of the output of the operation with
        // the mapped value of the "qin" operand of this operation
        rewriter.replaceAllUsesWith(op.getQout(), adaptor.getQin());
        auto newRotate = rewriter.create<qmem::RotateZOp>(op.getLoc(), adaptor.getQin(), adaptor.getAngle());
        // This is a tricky replacement.... we need to replace the operation *WITH THE VALUES OF THE OPERANDS*
        // which are the "modified" values on the qubits
        return RotateZLowering::NewOpAndValues(newRotate, ValueRange{newRotate.getQ()});
    }

    HadamardLowering::NewOpAndValues
    HadamardLowering::createNewOpAndValues(
            qnet::HadamardOp op, qnet::HadamardOp::Adaptor adaptor, ConversionPatternRewriter &rewriter) const {
        llvm::dbgs() << "lowering operation : '" << op << "'\n";
        // Since we move away from SSA, we need to replace all the uses of the output of the operation with
        // the mapped value of the "qin" operand of this operation
        rewriter.replaceAllUsesWith(op.getQout(), adaptor.getQin());
        auto newHadamard = rewriter.create<qmem::HadamardOp>(op.getLoc(), adaptor.getQin());
        // This is a tricky replacement.... we need to replace the operation *WITH THE VALUES OF THE OPERANDS*
        // which are the "modified" values on the qubits
        return HadamardLowering::NewOpAndValues(newHadamard, ValueRange{newHadamard.getQ()});
    }

    MeasureLowering::NewOpAndValues
    MeasureLowering::createNewOpAndValues(
            qnet::MeasureOp op, qnet::MeasureOp::Adaptor adaptor, ConversionPatternRewriter &rewriter) const {
        llvm::dbgs() << "lowering operation : '" << op << "'\n";
        // Measure yields an i1 type in both dialect spaces... this value does not need lowering, so we don't
        // need to remap the uses of the measure value.
        auto newMeasure = rewriter.create<qmem::MeasureOp>(op.getLoc(), adaptor.getQin0());
        // This is a tricky replacement.... we need to replace the operation *WITH THE VALUES OF THE OPERANDS*
        // which are the "modified" values on the qubits
        return MeasureLowering::NewOpAndValues(newMeasure, ValueRange{newMeasure.getQ()});
    }

    CNotLowering::NewOpAndValues
    CNotLowering::createNewOpAndValues(
            qnet::CnotOp op, qnet::CnotOp::Adaptor adaptor, ConversionPatternRewriter &rewriter) const {
        // Since we move away from SSA, we need to replace all the uses of the outputs of the operation with
        // the mapped value of the respective "qin" operand of this operation
        // NOTE - For some reason, if we use the rewriter object for this purpose, it ends up on a SIGSEGV
        // in the internals of the replacement of the operation
        op.getQout0().replaceAllUsesWith(adaptor.getQin0());
        op.getQout1().replaceAllUsesWith(adaptor.getQin1());
        auto newCnot = rewriter.create<qmem::CnotOp>(op.getLoc(), adaptor.getQin0(), adaptor.getQin1());
        // This is a tricky replacement.... we need to replace the operation *WITH THE VALUES OF THE OPERANDS*
        // which are the "modified" values on the qubits
        return CNotLowering::NewOpAndValues(newCnot, ValueRange{newCnot->getOperands()});
    }

    CzLowering::NewOpAndValues
    CzLowering::createNewOpAndValues(
            qnet::CzOp op, qnet::CzOp::Adaptor adaptor, ConversionPatternRewriter &rewriter) const {
        // Since we move away from SSA, we need to replace all the uses of the outputs of the operation with
        // the mapped value of the respective "qin" operand of this operation
        // NOTE - For some reason, if we use the rewriter object for this purpose, it ends up on a SIGSEGV
        // in the internals of the replacement of the operation
        op.getQout0().replaceAllUsesWith(adaptor.getQin0());
        op.getQout1().replaceAllUsesWith(adaptor.getQin1());
        auto newCz = rewriter.create<qmem::CzOp>(op.getLoc(), adaptor.getQin0(), adaptor.getQin1());
        // This is a tricky replacement.... we need to replace the operation *WITH THE VALUES OF THE OPERANDS*
        // which are the "modified" values on the qubits
        return CzLowering::NewOpAndValues(newCz, ValueRange{newCz->getOperands()});
    }

    CRotXLowering::NewOpAndValues
    CRotXLowering::createNewOpAndValues(
            qnet::CrotXOp op, qnet::CrotXOp::Adaptor adaptor, ConversionPatternRewriter &rewriter) const {
        // Since we move away from SSA, we need to replace all the uses of the outputs of the operation with
        // the mapped value of the respective "qin" operand of this operation
        // NOTE - For some reason, if we use the rewriter object for this purpose, it ends up on a SIGSEGV
        // in the internals of the replacement of the operation
        op.getQout0().replaceAllUsesWith(adaptor.getQin0());
        op.getQout1().replaceAllUsesWith(adaptor.getQin1());
        auto newCRotX = rewriter.create<qmem::CrotXOp>(op.getLoc(), adaptor.getQin0(), adaptor.getQin1(), adaptor.getAngle());
        // This is a tricky replacement.... we need to replace the operation *WITH THE VALUES OF THE OPERANDS*
        // which are the "modified" values on the qubits
        // In this particular case we only need the first 2 operands
        auto opOperands = newCRotX->getOpOperands();
        OperandRange firstTwoOperands(opOperands.data(), 2);
        return CRotXLowering::NewOpAndValues(newCRotX, ValueRange{firstTwoOperands});
    }

    /* Implementation of the specific conversion between similar ops
     * These implementations do not need any special treatment, and mapping is quite straight forward */
    qmem::RemoteOp
    RemoteOpLowering::createNewOp(qnet::RemoteOp op, qnet::RemoteOp::Adaptor adaptor,
                                  ConversionPatternRewriter &rewriter) const {
        return rewriter.create<qmem::RemoteOp>(op.getLoc(), adaptor.getSymName(), adaptor.getSymVisibilityAttr());
    }

    qmem::SendIntsOp
    SendIntsOpLowering::createNewOp(qnet::SendIntsOp op, qnet::SendIntsOp::Adaptor adaptor,
                                    ConversionPatternRewriter &rewriter) const {
        return rewriter.create<qmem::SendIntsOp>(op.getLoc(), adaptor.getCin(), adaptor.getRemoteAttr());
    }

    qmem::RecvIntsOp
    RecvIntsOpLowering::createNewOp(qnet::RecvIntsOp op, qnet::RecvIntsOp::Adaptor adaptor,
                                    ConversionPatternRewriter &rewriter) const {
        return rewriter.create<qmem::RecvIntsOp>(op.getLoc(), adaptor.getOperands(), adaptor.getRemoteAttr());
    }

    qmem::SendFloatsOp
    SendFloatsOpLowering::createNewOp(qnet::SendFloatsOp op, qnet::SendFloatsOp::Adaptor adaptor,
                                      ConversionPatternRewriter &rewriter) const {
        return rewriter.create<qmem::SendFloatsOp>(op.getLoc(), adaptor.getCin(), adaptor.getRemoteAttr());
    }

    qmem::RecvFloatsOp
    RecvFloatsOpLowering::createNewOp(qnet::RecvFloatsOp op, qnet::RecvFloatsOp::Adaptor adaptor,
                                      ConversionPatternRewriter &rewriter) const {
        return rewriter.create<qmem::RecvFloatsOp>(op.getLoc(), adaptor.getOperands(), adaptor.getRemoteAttr());
    }
} // namespace qoala::conversion