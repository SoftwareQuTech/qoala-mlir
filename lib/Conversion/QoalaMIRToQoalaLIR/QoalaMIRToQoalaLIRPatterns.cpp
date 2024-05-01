#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIRPatterns.h"

namespace qoala::conversion::mir {
    func::CallOp insertCallAngleTransform(Operation *operation, ConversionPatternRewriter &rewriter, Value angle) {
        // We also need to find the declaration of the angle conversion builtin
        auto module = operation->getParentOfType<ModuleOp>();
        auto angleConversionFunction = module.lookupSymbol<func::FuncOp>(angle::angleConversionFunctionName);
        // Create a call to the builtin
        return rewriter.create<func::CallOp>(operation->getLoc(), angleConversionFunction, angle);
    }

    /* Lowering for operations that belong in the module */
    qoalahost::RemoteOp
    RemoteOpLowering::createNewOp(qmem::RemoteOp op, qmem::RemoteOp::Adaptor adaptor,
                                  ConversionPatternRewriter &rewriter) const {
        return rewriter.create<qoalahost::RemoteOp>(
                op.getLoc(),
                adaptor.getSymNameAttr(),
                adaptor.getSymVisibilityAttr());
    }


    qoalahost::MainFuncOp
    FuncOpLowering::createNewOp(qmem::FuncOp op, qmem::FuncOp::Adaptor adaptor,
                                ConversionPatternRewriter &rewriter) const {
        return rewriter.create<qoalahost::MainFuncOp>(
                op.getLoc(),
                adaptor.getSymName(),
                adaptor.getFunctionType(),
                adaptor.getSymVisibilityAttr(),
                adaptor.getArgAttrsAttr(),
                adaptor.getResAttrsAttr());
    }

    qoalahost::ReturnOp
    ReturnOpLowering::createNewOp(qmem::ReturnOp op, qmem::ReturnOp::Adaptor adaptor,
                                  ConversionPatternRewriter &rewriter) const {
        return rewriter.create<qoalahost::ReturnOp>(op.getLoc(), adaptor.getOperands());
    }

    /* Lowering for operations that can only belong to netqasm.local_routine or netqasm.request_routine */
    netqasm::RotateXOp
    RotateXLowering::createNewOp(qmem::RotateXOp op, qmem::RotateXOp::Adaptor adaptor,
                                 ConversionPatternRewriter &rewriter) const {
        // Since we move away from SSA, we need to replace all the uses of the output of the operation with
        // the mapped value of the "qin" operand of this operation
        rewriter.replaceAllUsesWith(op.getQ(), adaptor.getQ());
        // The angle is a float, we need to transform it to 2 integers, using a builtin
        func::CallOp angleConversionCall = insertCallAngleTransform(op.getOperation(), rewriter, adaptor.getAngle());
        // And use the results of the conversion as the arguments of the new rotate operation
        return rewriter.create<netqasm::RotateXOp>(
                op.getLoc(), adaptor.getQ(),
                angleConversionCall.getResult(0), angleConversionCall.getResult(1));
    }

    netqasm::RotateYOp
    RotateYLowering::createNewOp(qmem::RotateYOp op, qmem::RotateYOp::Adaptor adaptor,
                                 ConversionPatternRewriter &rewriter) const {
        // Since we move away from SSA, we need to replace all the uses of the output of the operation with
        // the mapped value of the "qin" operand of this operation
        rewriter.replaceAllUsesWith(op.getQ(), adaptor.getQ());
        // The angle is a float, we need to transform it to 2 integers, using a builtin
        func::CallOp angleConversionCall = insertCallAngleTransform(op.getOperation(), rewriter, adaptor.getAngle());
        // And use the results of the conversion as the arguments of the new rotate operation
        return rewriter.create<netqasm::RotateYOp>(
                op.getLoc(), adaptor.getQ(),
                angleConversionCall.getResult(0), angleConversionCall.getResult(1));
    }

    netqasm::RotateZOp
    RotateZLowering::createNewOp(qmem::RotateZOp op, qmem::RotateZOp::Adaptor adaptor,
                                 ConversionPatternRewriter &rewriter) const {
        // Since we move away from SSA, we need to replace all the uses of the output of the operation with
        // the mapped value of the "qin" operand of this operation
        rewriter.replaceAllUsesWith(op.getQ(), adaptor.getQ());
        // The angle is a float, we need to transform it to 2 integers, using a builtin
        func::CallOp angleConversionCall = insertCallAngleTransform(op.getOperation(), rewriter, adaptor.getAngle());
        // And use the results of the conversion as the arguments of the new rotate operation
        return rewriter.create<netqasm::RotateZOp>(
                op.getLoc(), adaptor.getQ(),
                angleConversionCall.getResult(0), angleConversionCall.getResult(1));
    }

    netqasm::MeasureOp
    MeasureLowering::createNewOp(qmem::MeasureOp op, qmem::MeasureOp::Adaptor adaptor,
                                 ConversionPatternRewriter &rewriter) const {
        rewriter.replaceAllUsesWith(op.getQ(), adaptor.getQ());
        Type outcomeType = this->typeConverter->convertType(adaptor.getQ().getType());
        return rewriter.create<netqasm::MeasureOp>(op.getLoc(), outcomeType, adaptor.getQ());
    }

    netqasm::HadamardOp
    HadamardLowering::createNewOp(qmem::HadamardOp op, qmem::HadamardOp::Adaptor adaptor,
                                  ConversionPatternRewriter &rewriter) const {
        rewriter.replaceAllUsesWith(op.getQ(), adaptor.getQ());
        return rewriter.create<netqasm::HadamardOp>(op.getLoc(), adaptor.getQ());
    }

    netqasm::CnotOp
    CNotLowering::createNewOp(qmem::CnotOp op, qmem::CnotOp::Adaptor adaptor,
                                  ConversionPatternRewriter &rewriter) const {
        rewriter.replaceAllUsesWith(op.getQin0(), adaptor.getQin0());
        rewriter.replaceAllUsesWith(op.getQin1(), adaptor.getQin1());
        return rewriter.create<netqasm::CnotOp>(op.getLoc(), adaptor.getQin0(), adaptor.getQin0());
    }

    netqasm::CzOp
    CzLowering::createNewOp(qmem::CzOp op, qmem::CzOp::Adaptor adaptor,
                            mlir::ConversionPatternRewriter &rewriter) const {
        rewriter.replaceAllUsesWith(op.getQin0(), adaptor.getQin0());
        rewriter.replaceAllUsesWith(op.getQin1(), adaptor.getQin1());
        return rewriter.create<netqasm::CzOp>(op.getLoc(), adaptor.getQin0(), adaptor.getQin0());
    }

    netqasm::CrotXOp
    CRotXLowering::createNewOp(qmem::CrotXOp op, qmem::CrotXOp::Adaptor adaptor,
                               mlir::ConversionPatternRewriter &rewriter) const {
        rewriter.replaceAllUsesWith(op.getQin0(), adaptor.getQin0());
        rewriter.replaceAllUsesWith(op.getQin1(), adaptor.getQin1());
        // The angle is a float, we need to transform it to 2 integers, using a builtin
        func::CallOp angleConversionCall = insertCallAngleTransform(op.getOperation(), rewriter, adaptor.getAngle());
        // And use the results of the conversion as the arguments of the new rotate operation
        return rewriter.create<netqasm::CrotXOp>(
                op.getLoc(), adaptor.getQin0(), adaptor.getQin0(),
                angleConversionCall.getOperand(0), angleConversionCall.getOperand(1));
    }
} // namespace qoala::conversion::mir