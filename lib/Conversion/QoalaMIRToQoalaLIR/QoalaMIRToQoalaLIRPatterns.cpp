#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIRPatterns.h"
#include "llvm/Support/Debug.h"

namespace qoala::conversion::mir {
    func::CallOp insertCallAngleTransform(Operation *operation, ConversionPatternRewriter &rewriter, Value angle) {
        // We also need to find the declaration of the angle conversion builtin
        auto module = operation->getParentOfType<ModuleOp>();
        auto angleConversionFunction = module.lookupSymbol<func::FuncOp>(angle::angleConversionFunctionName);
        // Create a call to the builtin
        return rewriter.create<func::CallOp>(operation->getLoc(), angleConversionFunction, angle);
    }

    /* Lowering for operations define the main function or are inside it - Will map to QoalaHost dialect */
    ValueRange
    RemoteOpLowering::createNewOpAndValues(qmem::RemoteOp op, qmem::RemoteOp::Adaptor adaptor,
                                           ConversionPatternRewriter &rewriter) const {
        auto newReturn = rewriter.create<qoalahost::RemoteOp>(
                op.getLoc(),
                adaptor.getSymNameAttr(),
                adaptor.getSymVisibilityAttr());
        return newReturn->getResults();
    }


    ValueRange
    FuncOpLowering::createNewOpAndValues(qmem::FuncOp op, qmem::FuncOp::Adaptor adaptor,
                                         ConversionPatternRewriter &rewriter) const {
        auto newFunc = rewriter.create<qoalahost::MainFuncOp>(
                op.getLoc(),
                adaptor.getSymName(),
                adaptor.getFunctionType(),
                adaptor.getSymVisibilityAttr(),
                adaptor.getArgAttrsAttr(),
                adaptor.getResAttrsAttr());
        rewriter.inlineRegionBefore(op.getFunctionBody(), newFunc.getBody(), newFunc.end());
        return newFunc->getResults();
    }

    ValueRange
    ReturnOpLowering::createNewOpAndValues(qmem::ReturnOp op, qmem::ReturnOp::Adaptor adaptor,
                                           ConversionPatternRewriter &rewriter) const {
        auto newReturn = rewriter.create<qoalahost::ReturnOp>(
                op.getLoc(),
                adaptor.getOperands());
        return newReturn->getResults();
    }

    ValueRange
    CallOpLowering::createNewOpAndValues(func::CallOp op, func::CallOp::Adaptor adaptor,
                                         ConversionPatternRewriter &rewriter) const {
        auto newCall = rewriter.create<qoalahost::CallOp>(
                op.getLoc(),
                adaptor.getCallee(),
                op->getResultTypes(),
                op.getOperands());
        return newCall->getResults();
    }

    ValueRange
    RecvIntsOpLowering::createNewOpAndValues(qmem::RecvIntsOp op, qmem::RecvIntsOp::Adaptor adaptor,
                                             ConversionPatternRewriter &rewriter) const {
        Type convertedType = this->typeConverter->convertType(op.getCout().getType());
        auto newRecv = rewriter.create<qoalahost::RecvIntsOp>(
                op.getLoc(),
                convertedType,
                adaptor.getRemoteAttr());
        return newRecv->getResults();
    }

    ValueRange
    RecvFloatsOpLowering::createNewOpAndValues(qmem::RecvFloatsOp op, qmem::RecvFloatsOp::Adaptor adaptor,
                                               ConversionPatternRewriter &rewriter) const {
        Type convertedType = this->typeConverter->convertType(op.getCout().getType());
        auto newRecv = rewriter.create<qoalahost::RecvFloatsOp>(
                op.getLoc(),
                convertedType,
                adaptor.getRemoteAttr());
        return newRecv->getResults();
    }

    /* Lowering for operations that define or are inside local_routine or request_routine - Will map to NetQASM dialect */
    ValueRange
    MeasureOpLowering::createNewOpAndValues(qmem::MeasureOp op, qmem::MeasureOp::Adaptor adaptor,
                                            ConversionPatternRewriter &rewriter) const {
        auto newOp = rewriter.create<netqasm::MeasureOp>(op.getLoc(), rewriter.getI1Type(), adaptor.getQ());
        return newOp->getResults();
    }

    ValueRange
    EprsOpLowering::createNewOpAndValues(qmem::EprsOp op, qmem::EprsOp::Adaptor adaptor,
                                         ConversionPatternRewriter &rewriter) const {
        auto newEprs = rewriter.create<netqasm::EprsOp>(
                op.getLoc(),
                adaptor.getQ(),
                adaptor.getRemoteAttr());
        return newEprs->getResults();
    }

    ValueRange
    EprsMeasureOpLowering::createNewOpAndValues(qmem::EprsMeasureOp op, qmem::EprsMeasureOp::Adaptor adaptor,
                                                ConversionPatternRewriter &rewriter) const {
        auto newEprs = rewriter.create<netqasm::EprsMeasureOp>(op.getLoc(), rewriter.getI1Type(),
                                                       adaptor.getQ(), adaptor.getRemoteAttr());
        return newEprs->getResults();
    }

    ValueRange
    NetQASMFunctionLowering::createNewOpAndValues(func::FuncOp op, func::FuncOp::Adaptor adaptor,
                                                  ConversionPatternRewriter &rewriter) const {
        StringAttr entangleAttr = dyn_cast_or_null<StringAttr>(op->getAttr("entangle"));
        if (entangleAttr) {
            auto newFunc = rewriter.create<netqasm::RequestRoutineOp>(
                    op.getLoc(),
                    adaptor.getSymName(),
                    adaptor.getFunctionType(),
                    adaptor.getSymVisibilityAttr(),
                    adaptor.getArgAttrsAttr(),
                    adaptor.getResAttrsAttr());
            rewriter.inlineRegionBefore(op.getFunctionBody(), newFunc.getBody(), newFunc.end());
            // TODO - Analyze the copied body and determine statistics, such as used and maintained qubits
            return newFunc->getResults();
        } else {
            auto newFunc = rewriter.create<netqasm::LocalRoutineOp>(
                    op.getLoc(),
                    adaptor.getSymName(),
                    adaptor.getFunctionType(),
                    adaptor.getSymVisibilityAttr(),
                    adaptor.getArgAttrsAttr(),
                    adaptor.getResAttrsAttr());
            rewriter.inlineRegionBefore(op.getFunctionBody(), newFunc.getBody(), newFunc.end());
            // TODO - Analyze the copied body and determine statistics, such as used and maintained qubits
            return newFunc->getResults();
        }
    }

    ValueRange
    NetQASMReturnOpLowering::createNewOpAndValues(func::ReturnOp op, func::ReturnOp::Adaptor adaptor,
                                                  ConversionPatternRewriter &rewriter) const {
        auto newReturn = rewriter.create<netqasm::ReturnOp>(op.getLoc(), adaptor.getOperands());
        return newReturn->getResults();
    }

    ValueRange
    QAllocLowering::createNewOpAndValues(qmem::QAllocOp op, qmem::QAllocOp::Adaptor adaptor,
                                         ConversionPatternRewriter &rewriter) const {
        auto newAlloc = rewriter.create<netqasm::QAllocOp>(op.getLoc());
        return newAlloc->getResults();
    }

    ValueRange
    QInitLowering::createNewOpAndValues(qmem::InitOp op, qmem::InitOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const {
        auto newQInit = rewriter.create<netqasm::QInitOp>(op.getLoc(), adaptor.getQ());
        return newQInit->getResults();
    }

    ValueRange
    RotateXLowering::createNewOpAndValues(qmem::RotateXOp op, qmem::RotateXOp::Adaptor adaptor,
                                          ConversionPatternRewriter &rewriter) const {
        // The angle is a float, we need to transform it to 2 integers, using a builtin
        func::CallOp angleConversionCall = insertCallAngleTransform(op.getOperation(), rewriter, adaptor.getAngle());
        // And use the results of the conversion as the arguments of the new rotate operation
        auto newRotate = rewriter.create<netqasm::RotateXOp>(
                op.getLoc(), adaptor.getQ(),
                angleConversionCall.getResult(0), angleConversionCall.getResult(1));
        return newRotate->getResults();
    }

    ValueRange
    RotateYLowering::createNewOpAndValues(qmem::RotateYOp op, qmem::RotateYOp::Adaptor adaptor,
                                          ConversionPatternRewriter &rewriter) const {
        // The angle is a float, we need to transform it to 2 integers, using a builtin
        func::CallOp angleConversionCall = insertCallAngleTransform(op.getOperation(), rewriter, adaptor.getAngle());
        // And use the results of the conversion as the arguments of the new rotate operation
        auto newRotate = rewriter.create<netqasm::RotateYOp>(
                op.getLoc(), adaptor.getQ(),
                angleConversionCall.getResult(0), angleConversionCall.getResult(1));
        return newRotate->getResults();
    }

    ValueRange
    RotateZLowering::createNewOpAndValues(qmem::RotateZOp op, qmem::RotateZOp::Adaptor adaptor,
                                          ConversionPatternRewriter &rewriter) const {
        // The angle is a float, we need to transform it to 2 integers, using a builtin
        func::CallOp angleConversionCall = insertCallAngleTransform(op.getOperation(), rewriter, adaptor.getAngle());
        // And use the results of the conversion as the arguments of the new rotate operation
        auto newRotate= rewriter.create<netqasm::RotateZOp>(
                op.getLoc(), adaptor.getQ(),
                angleConversionCall.getResult(0), angleConversionCall.getResult(1));
        return newRotate->getResults();
    }

    ValueRange
    HadamardLowering::createNewOpAndValues(qmem::HadamardOp op, qmem::HadamardOp::Adaptor adaptor,
                                           ConversionPatternRewriter &rewriter) const {
        auto newHadamard = rewriter.create<netqasm::HadamardOp>(op.getLoc(), adaptor.getQ());
        return newHadamard->getResults();
    }

    ValueRange
    CNotLowering::createNewOpAndValues(qmem::CnotOp op, qmem::CnotOp::Adaptor adaptor,
                                       ConversionPatternRewriter &rewriter) const {
        auto newCnot = rewriter.create<netqasm::CnotOp>(op.getLoc(), adaptor.getQin0(), adaptor.getQin0());
        return newCnot->getResults();
    }

    ValueRange
    CzLowering::createNewOpAndValues(qmem::CzOp op, qmem::CzOp::Adaptor adaptor,
                                     ConversionPatternRewriter &rewriter) const {
        auto newCz = rewriter.create<netqasm::CzOp>(op.getLoc(), adaptor.getQin0(), adaptor.getQin0());
        return newCz->getResults();
    }

    ValueRange
    CRotXLowering::createNewOpAndValues(qmem::CrotXOp op, qmem::CrotXOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const {
        // The angle is a float, we need to transform it to 2 integers, using a builtin
        func::CallOp angleConversionCall = insertCallAngleTransform(op.getOperation(), rewriter, adaptor.getAngle());
        // And use the results of the conversion as the arguments of the new rotate operation
        auto newCrotX = rewriter.create<netqasm::CrotXOp>(
                op.getLoc(), adaptor.getQin0(), adaptor.getQin0(),
                angleConversionCall.getOperand(0), angleConversionCall.getOperand(1));
        return newCrotX->getResults();
    }
} // namespace qoala::conversion::mir