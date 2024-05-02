#include "Conversion/Helpers/Helpers.h"
#include "Conversion/QoalaHIRToQoalaMIR/QoalaHIRToQoalaMIRPatterns.h"

namespace qoala::conversion::hir {
    /* Implementation of the qoala types converter */
    QoalaHIRToQoalaMIRTypeConverter::QoalaHIRToQoalaMIRTypeConverter(MLIRContext *ctx) {
        // Default conversion for non qnet::QubitType instances
        addConversion([](Type type) { return type; });
        addConversion([ctx](qnet::QubitType type) -> Type {
            // Qubit Types are mapped into i32 (pointers)
            return IntegerType::get(ctx, 32);
        });
        // In case there will be illegal values (values of types declared illegal)
        // after conversion, then we need to provide a "materialization", i.e. how
        // to cast from one type to the other Here we provide a "cast" to transform
        // from source type (qnet::QubitType) to target type (qmem::QubitType = i32)
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

    ValueRange
    FuncOpLowering::createNewOpAndValues(qnet::FuncOp op, qnet::FuncOp::Adaptor adaptor,
                                         ConversionPatternRewriter &rewriter) const {
        auto newFunc = rewriter.create<qmem::FuncOp>(op.getLoc(), adaptor.getSymName(),
                                                     adaptor.getFunctionType(),
                                                     adaptor.getAttributes().getValue(),
                                                     ArrayRef<DictionaryAttr>{});
        // Before returning the new function, we need to "attach" (inline)
        // the body (region) of the old operation to the new one
        // This is inspired in the last part of the `convertFuncOpToLLVMFuncOp`
        // function in llvm/mlir/lib/Conversion/FuncToLLVM/FuncToLLVM.cpp
        rewriter.inlineRegionBefore(op.getFunctionBody(), newFunc.getBody(), newFunc.end());
        return newFunc->getResults();
    }

    ValueRange
    ReturnOpLowering::createNewOpAndValues(qnet::ReturnOp op, qnet::ReturnOp::Adaptor adaptor,
                                           ConversionPatternRewriter &rewriter) const {
        auto newReturn = rewriter.create<qmem::ReturnOp>(op.getLoc(), adaptor.getOperands());
        return newReturn->getResults();
    }

    /* Implementation of the operations that entangle qubits */
    ValueRange
    EprsOpLowering::createNewOpAndValues(qnet::EprsOp op, qnet::EprsOp::Adaptor adaptor,
                                         ConversionPatternRewriter &rewriter) const {
        Location loc = op.getLoc();
        // We first create a new qalloc operation
        auto newAllocOp = rewriter.create<qmem::QAllocOp>(loc);

        // Then we introduce the "init" operation as the "eprs" op
        auto newEprsOp = rewriter.create<qmem::EprsOp>(loc, newAllocOp.getResult(), adaptor.getRemoteAttr());
        // We replace all the uses of the old qubit with the new value (what was passed as "q" operand to eprs)
        rewriter.replaceAllUsesWith(op.getQout(), newEprsOp.getQ());
        return ValueRange{newAllocOp.getResult()};
    }

    ValueRange
    EprsMeasureOpLowering::createNewOpAndValues(qnet::EprsMeasureOp op, qnet::EprsMeasureOp::Adaptor adaptor,
                                                ConversionPatternRewriter &rewriter) const {
        Location loc = op.getLoc();
        // We first create a new qalloc operation
        auto newAllocOp = rewriter.create<qmem::QAllocOp>(loc);
        Type mappedOutType = typeConverter->convertType(op.getOutcome().getType());
        auto newEprs = rewriter.create<qmem::EprsMeasureOp>(
                loc,
                mappedOutType,
                newAllocOp.getQ(),
                adaptor.getRemoteAttr());
        return newEprs->getResults();
    }

    /* Implementation of the lowering for the creation of new qubits */
    ValueRange
    NewQubitLowering::createNewOpAndValues(qnet::NewQubitOp op, qnet::NewQubitOp::Adaptor adaptor,
                                           ConversionPatternRewriter &rewriter) const {
        Location loc = op.getLoc();
        // First, we convert the qnet.qubit type into its mapped type
        Type mappedQubitType = typeConverter->convertType(op.getQout().getType());

        // Second, we create a QAllocOp instance that allocates a single qubit
        auto newAllocOp = rewriter.create<qmem::QAllocOp>(loc, mappedQubitType);

        // Third, we introduce the "init" operation required by the allocation
        auto newInitOp = rewriter.create<qmem::InitOp>(loc, newAllocOp.getResult());
        auto qalloc = dyn_cast<qmem::QAllocOp>(newInitOp.getQ().getDefiningOp());
        return qalloc->getResults();
    }

    ValueRange
    RotateXLowering::createNewOpAndValues(qnet::RotXOp op, qnet::RotXOp::Adaptor adaptor,
                                          ConversionPatternRewriter &rewriter) const {
        // Since we move away from SSA, we need to replace all the uses of the output of the operation with
        // the mapped value of the "qin" operand of this operation
        rewriter.replaceAllUsesWith(op.getQout(), adaptor.getQin());
        auto newRotate = rewriter.create<qmem::RotateXOp>(op.getLoc(), adaptor.getQin(), adaptor.getAngle());
        // This is a tricky replacement.... we need to replace the operation *WITH THE VALUES OF THE OPERANDS*
        // which are the "modified" values on the qubits
        return ValueRange{newRotate.getQ()};
    }

    ValueRange
    RotateYLowering::createNewOpAndValues(qnet::RotYOp op, qnet::RotYOp::Adaptor adaptor,
                                          ConversionPatternRewriter &rewriter) const {
        // Since we move away from SSA, we need to replace all the uses of the output of the operation with
        // the mapped value of the "qin" operand of this operation
        rewriter.replaceAllUsesWith(op.getQout(), adaptor.getQin());
        auto newRotate = rewriter.create<qmem::RotateYOp>(op.getLoc(), adaptor.getQin(), adaptor.getAngle());
        // This is a tricky replacement.... we need to replace the operation *WITH THE VALUES OF THE OPERANDS*
        // which are the "modified" values on the qubits
        return ValueRange{newRotate.getQ()};
    }

    ValueRange
    RotateZLowering::createNewOpAndValues(qnet::RotZOp op, qnet::RotZOp::Adaptor adaptor,
                                          ConversionPatternRewriter &rewriter) const {
        // Since we move away from SSA, we need to replace all the uses of the output of the operation with
        // the mapped value of the "qin" operand of this operation
        rewriter.replaceAllUsesWith(op.getQout(), adaptor.getQin());
        auto newRotate = rewriter.create<qmem::RotateZOp>(op.getLoc(), adaptor.getQin(), adaptor.getAngle());
        // This is a tricky replacement.... we need to replace the operation *WITH THE VALUES OF THE OPERANDS*
        // which are the "modified" values on the qubits
        return ValueRange{newRotate.getQ()};
    }

    ValueRange
    HadamardLowering::createNewOpAndValues(qnet::HadamardOp op, qnet::HadamardOp::Adaptor adaptor,
                                           ConversionPatternRewriter &rewriter) const {
        // Since we move away from SSA, we need to replace all the uses of the output of the operation with
        // the mapped value of the "qin" operand of this operation
        rewriter.replaceAllUsesWith(op.getQout(), adaptor.getQin());
        auto newHadamard = rewriter.create<qmem::HadamardOp>(op.getLoc(), adaptor.getQin());
        // This is a tricky replacement.... we need to replace the operation *WITH THE VALUES OF THE OPERANDS*
        // which are the "modified" values on the qubits
        return ValueRange{newHadamard.getQ()};
    }

    ValueRange
    MeasureLowering::createNewOpAndValues(qnet::MeasureOp op, qnet::MeasureOp::Adaptor adaptor,
                                          ConversionPatternRewriter &rewriter) const {
        // Measure yields an i1 type in both dialect spaces... this value does not need lowering, so we don't
        // need to remap the uses of the measure value.
        auto newMeasure = rewriter.create<qmem::MeasureOp>(op.getLoc(), rewriter.getI1Type(), adaptor.getQin());
        // This is a tricky replacement.... we need to replace the operation *WITH THE VALUES OF THE OPERANDS*
        // which are the "modified" values on the qubits
        return ValueRange{adaptor.getQin()};
    }

    ValueRange
    CNotLowering::createNewOpAndValues(qnet::CnotOp op, qnet::CnotOp::Adaptor adaptor,
                                       ConversionPatternRewriter &rewriter) const {
        // Since we move away from SSA, we need to replace all the uses of the outputs of the operation with
        // the mapped value of the respective "qin" operand of this operation
        // NOTE - For some reason (probably a misuse of MLIR or a bug) , if we use the rewriter object for
        // this purpose, it ends up on a SIGSEGV in the internals of the replacement of the operation
        // By using the "replaceAllUsesWith" directly from the value to replace, makes it work correctlyW
        op.getQout0().replaceAllUsesWith(adaptor.getQin0());
        op.getQout1().replaceAllUsesWith(adaptor.getQin1());
        auto newCnot = rewriter.create<qmem::CnotOp>(op.getLoc(), adaptor.getQin0(), adaptor.getQin1());
        // This is a tricky replacement.... we need to replace the operation *WITH THE VALUES OF THE OPERANDS*
        // which are the "modified" values on the qubits
        return ValueRange{newCnot->getOperands()};
    }

    ValueRange
    CzLowering::createNewOpAndValues(qnet::CzOp op, qnet::CzOp::Adaptor adaptor,
                                     ConversionPatternRewriter &rewriter) const {
        // Since we move away from SSA, we need to replace all the uses of the outputs of the operation with
        // the mapped value of the respective "qin" operand of this operation
        // NOTE - For some reason, if we use the rewriter object for this purpose, it ends up on a SIGSEGV
        // in the internals of the replacement of the operation
        op.getQout0().replaceAllUsesWith(adaptor.getQin0());
        op.getQout1().replaceAllUsesWith(adaptor.getQin1());
        auto newCz = rewriter.create<qmem::CzOp>(op.getLoc(), adaptor.getQin0(), adaptor.getQin1());
        // This is a tricky replacement.... we need to replace the operation *WITH THE VALUES OF THE OPERANDS*
        // which are the "modified" values on the qubits
        return ValueRange{newCz->getOperands()};
    }

    ValueRange
    CRotXLowering::createNewOpAndValues(qnet::CrotXOp op, qnet::CrotXOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const {
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
        return ValueRange{firstTwoOperands};
    }

    /* Implementation of the specific conversion between similar ops
     * These implementations do not need any special treatment, and mapping is quite straight forward */
    ValueRange
    RemoteOpLowering::createNewOpAndValues(qnet::RemoteOp op, qnet::RemoteOp::Adaptor adaptor,
                                           ConversionPatternRewriter &rewriter) const {
        auto newRemote = rewriter.create<qmem::RemoteOp>(
                op.getLoc(), adaptor.getSymName(),
                adaptor.getSymVisibilityAttr());
        return newRemote->getResults();
    }

    ValueRange
    SendIntsOpLowering::createNewOpAndValues(qnet::SendIntsOp op, qnet::SendIntsOp::Adaptor adaptor,
                                             ConversionPatternRewriter &rewriter) const {
        auto newSend = rewriter.create<qmem::SendIntsOp>(
                op.getLoc(), adaptor.getCin(),
                adaptor.getRemoteAttr());
        return newSend->getResults();
    }

    ValueRange
    RecvIntsOpLowering::createNewOpAndValues(qnet::RecvIntsOp op, qnet::RecvIntsOp::Adaptor adaptor,
                                             ConversionPatternRewriter &rewriter) const {
        // The output type is unchanged by this conversion;we will pass it "as is" to the new operation
        Type outType = typeConverter->convertType(op.getCout().getType());
        auto newRec = rewriter.create<qmem::RecvIntsOp>(
                op.getLoc(), outType,
                adaptor.getRemoteAttr(), adaptor.getLengthAttr());
        return newRec->getResults();
    }

    ValueRange
    SendFloatsOpLowering::createNewOpAndValues(qnet::SendFloatsOp op, qnet::SendFloatsOp::Adaptor adaptor,
                                               ConversionPatternRewriter &rewriter) const {
        auto newSend = rewriter.create<qmem::SendFloatsOp>(
                op.getLoc(), adaptor.getCin(),
                adaptor.getRemoteAttr());
        return newSend->getResults();
    }

    ValueRange
    RecvFloatsOpLowering::createNewOpAndValues(qnet::RecvFloatsOp op, qnet::RecvFloatsOp::Adaptor adaptor,
                                               ConversionPatternRewriter &rewriter) const {
        // The output type is unchanged by this conversion;we will pass it "as is" to the new operation
        Type outType = typeConverter->convertType(op.getCout().getType());
        auto newSend = rewriter.create<qmem::RecvFloatsOp>(
                op.getLoc(), outType,
                adaptor.getRemoteAttr(), adaptor.getLengthAttr());
        return newSend->getResults();
    }
} // namespace qoala::conversion::hir