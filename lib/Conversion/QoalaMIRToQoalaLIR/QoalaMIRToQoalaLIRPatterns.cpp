#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Vector/Transforms/VectorRewritePatterns.h"

#include "Analysis/QoalaHost/Helpers.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIRPatterns.h"

#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "mir-to-lir-pattens"

using namespace mlir;
using namespace qoala::dialects;
using namespace qoala::helpers;

namespace qoala::conversion::mir {
    /* Lowering for operations define the main function or are inside it - Will map to QoalaHost dialect */
    std::unique_ptr<OpAndValues>
    RemoteOpLowering::createNewOpAndValues(qmem::RemoteOp op, qmem::RemoteOp::Adaptor adaptor,
                                           ConversionPatternRewriter &rewriter) const {
        auto newReturn = rewriter.create<qremote::RemoteOp>(
                op.getLoc(),
                adaptor.getSymNameAttr(),
                adaptor.getSymVisibilityAttr());
        return std::make_unique<OpAndValues>(newReturn.getOperation(), newReturn->getResults());
    }

    std::unique_ptr<OpAndValues>
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
        // After we create the new MainFuncOp, we will isolate the functions that need to be in a single block
        analysis::isolate::isolateOps(newFunc.getOperation(), rewriter);
        return std::make_unique<OpAndValues>(newFunc.getOperation(), newFunc->getResults());
    }

    std::unique_ptr<OpAndValues>
    ReturnOpLowering::createNewOpAndValues(qmem::ReturnOp op, qmem::ReturnOp::Adaptor adaptor,
                                           ConversionPatternRewriter &rewriter) const {
        auto newReturn = rewriter.create<qoalahost::ReturnOp>(
                op.getLoc(),
                adaptor.getOperands());
        return std::make_unique<OpAndValues>(newReturn.getOperation(), newReturn->getResults());
    }

    std::unique_ptr<OpAndValues>
    CallOpLowering::createNewOpAndValues(func::CallOp op, func::CallOp::Adaptor adaptor,
                                         ConversionPatternRewriter &rewriter) const {
        auto newCall = rewriter.create<qoalahost::CallOp>(
                op.getLoc(),
                adaptor.getCallee(),
                op->getResultTypes(),
                op.getOperands());
        return std::make_unique<OpAndValues>(newCall.getOperation(), newCall->getResults());
    }

    std::unique_ptr<OpAndValues>
    RecvIntsOpLowering::createNewOpAndValues(qmem::RecvIntsOp op, qmem::RecvIntsOp::Adaptor adaptor,
                                             ConversionPatternRewriter &rewriter) const {
        Type convertedType = this->typeConverter->convertType(op.getCout().getType());
        auto newRecv = rewriter.create<qoalahost::RecvIntsOp>(
                op.getLoc(),
                convertedType,
                adaptor.getRemoteAttr(),
                adaptor.getLengthAttr());
        return std::make_unique<OpAndValues>(newRecv.getOperation(), newRecv->getResults());
    }

    std::unique_ptr<OpAndValues>
    RecvFloatsOpLowering::createNewOpAndValues(qmem::RecvFloatsOp op, qmem::RecvFloatsOp::Adaptor adaptor,
                                               ConversionPatternRewriter &rewriter) const {
        Type convertedType = this->typeConverter->convertType(op.getCout().getType());
        auto newRecv = rewriter.create<qoalahost::RecvFloatsOp>(
                op.getLoc(),
                convertedType,
                adaptor.getRemoteAttr(),
                adaptor.getLengthAttr());
        return std::make_unique<OpAndValues>(newRecv.getOperation(), newRecv->getResults());
    }

    std::unique_ptr<OpAndValues>
    SendIntsOpLowering::createNewOpAndValues(qmem::SendIntsOp op, qmem::SendIntsOp::Adaptor adaptor,
                                             ConversionPatternRewriter &rewriter) const {
        auto newRecv = rewriter.create<qoalahost::SendIntsOp>(
                op.getLoc(),
                op.getCin(),
                adaptor.getRemoteAttr());
        return std::make_unique<OpAndValues>(newRecv.getOperation(), newRecv->getResults());
    }

    std::unique_ptr<OpAndValues>
    SendFloatsOpLowering::createNewOpAndValues(qmem::SendFloatsOp op, qmem::SendFloatsOp::Adaptor adaptor,
                                               ConversionPatternRewriter &rewriter) const {
        auto newRecv = rewriter.create<qoalahost::SendFloatsOp>(
                op.getLoc(),
                op.getCin(),
                adaptor.getRemoteAttr());
        return std::make_unique<OpAndValues>(newRecv.getOperation(), newRecv->getResults());
    }

    /* Lowering for operations that define or are inside local_routine or request_routine - Will map to NetQASM dialect */
    std::unique_ptr<OpAndValues>
    MeasureOpLowering::createNewOpAndValues(qmem::MeasureOp op, qmem::MeasureOp::Adaptor adaptor,
                                            ConversionPatternRewriter &rewriter) const {
        auto newOp = rewriter.create<netqasm::MeasureOp>(op.getLoc(), rewriter.getI1Type(), adaptor.getQ());
        return std::make_unique<OpAndValues>(newOp.getOperation(), newOp->getResults());
    }

    std::unique_ptr<OpAndValues>
    EprsOpLowering::createNewOpAndValues(qmem::EprsOp op, qmem::EprsOp::Adaptor adaptor,
                                         ConversionPatternRewriter &rewriter) const {
        auto newEprs = rewriter.create<netqasm::EprsOp>(
                op.getLoc(),
                adaptor.getQ(),
                adaptor.getRemoteAttr());
        return std::make_unique<OpAndValues>(newEprs.getOperation(), newEprs->getResults());
    }

    std::unique_ptr<OpAndValues>
    EprsMeasureOpLowering::createNewOpAndValues(qmem::EprsMeasureOp op, qmem::EprsMeasureOp::Adaptor adaptor,
                                                ConversionPatternRewriter &rewriter) const {
        auto newEprs = rewriter.create<netqasm::EprsMeasureOp>(op.getLoc(), rewriter.getI1Type(),
                                                       adaptor.getQ(), adaptor.getRemoteAttr());
        return std::make_unique<OpAndValues>(newEprs.getOperation(), newEprs->getResults());
    }

    std::unique_ptr<OpAndValues>
    NetQASMFunctionLowering::createNewOpAndValues(func::FuncOp op, func::FuncOp::Adaptor adaptor,
                                                  ConversionPatternRewriter &rewriter) const {
        if (dyn_cast_or_null<StringAttr>(op->getAttr("entangle"))) {
            auto newFunc = rewriter.create<netqasm::RequestRoutineOp>(
                    op.getLoc(),
                    adaptor.getSymName(),
                    adaptor.getFunctionType(),
                    adaptor.getSymVisibilityAttr(),
                    adaptor.getArgAttrsAttr(),
                    adaptor.getResAttrsAttr());
            rewriter.inlineRegionBefore(op.getFunctionBody(), newFunc.getBody(), newFunc.end());
            // TODO - Analyze the copied body and determine statistics, such as used and maintained qubits
            return std::make_unique<OpAndValues>(newFunc.getOperation(), newFunc->getResults());

        }
        auto newFunc = rewriter.create<netqasm::LocalRoutineOp>(
                op.getLoc(),
                adaptor.getSymName(),
                adaptor.getFunctionType(),
                adaptor.getSymVisibilityAttr(),
                adaptor.getArgAttrsAttr(),
                adaptor.getResAttrsAttr());
        rewriter.inlineRegionBefore(op.getFunctionBody(), newFunc.getBody(), newFunc.end());
        // TODO - Analyze the copied body and determine statistics, such as used and maintained qubits
        return std::make_unique<OpAndValues>(newFunc.getOperation(), newFunc->getResults());
    }

    std::unique_ptr<OpAndValues>
    NetQASMReturnOpLowering::createNewOpAndValues(func::ReturnOp op, func::ReturnOp::Adaptor adaptor,
                                                  ConversionPatternRewriter &rewriter) const {
        auto newReturn = rewriter.create<netqasm::ReturnOp>(op.getLoc(), adaptor.getOperands());
        return std::make_unique<OpAndValues>(newReturn.getOperation(), newReturn->getResults());
    }

    std::unique_ptr<OpAndValues>
    QAllocLowering::createNewOpAndValues(qmem::QAllocOp op, qmem::QAllocOp::Adaptor adaptor,
                                         ConversionPatternRewriter &rewriter) const {
        auto newAlloc = rewriter.create<netqasm::QAllocOp>(op.getLoc());
        return std::make_unique<OpAndValues>(newAlloc.getOperation(), newAlloc->getResults());
    }

    std::unique_ptr<OpAndValues>
    QInitLowering::createNewOpAndValues(qmem::InitOp op, qmem::InitOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const {
        auto newQInit = rewriter.create<netqasm::QInitOp>(op.getLoc(), adaptor.getQ());
        return std::make_unique<OpAndValues>(newQInit.getOperation(), newQInit->getResults());
    }

    std::unique_ptr<OpAndValues>
    HadamardLowering::createNewOpAndValues(qmem::HadamardOp op, qmem::HadamardOp::Adaptor adaptor,
                                           ConversionPatternRewriter &rewriter) const {
        auto newHadamard = rewriter.create<netqasm::HadamardOp>(op.getLoc(), adaptor.getQ());
        return std::make_unique<OpAndValues>(newHadamard.getOperation(), newHadamard->getResults());
    }

    std::unique_ptr<OpAndValues>
    CNotLowering::createNewOpAndValues(qmem::CnotOp op, qmem::CnotOp::Adaptor adaptor,
                                       ConversionPatternRewriter &rewriter) const {
        auto newCnot = rewriter.create<netqasm::CnotOp>(op.getLoc(), adaptor.getQin0(), adaptor.getQin1());
        return std::make_unique<OpAndValues>(newCnot.getOperation(), newCnot->getResults());
    }

    std::unique_ptr<OpAndValues>
    CzLowering::createNewOpAndValues(qmem::CzOp op, qmem::CzOp::Adaptor adaptor,
                                     ConversionPatternRewriter &rewriter) const {
        auto newCz = rewriter.create<netqasm::CzOp>(op.getLoc(), adaptor.getQin0(), adaptor.getQin1());
        return std::make_unique<OpAndValues>(newCz.getOperation(), newCz->getResults());
    }

    /* Lowering for "intermediate" operations that use the angle_num and angle_denom integers instead of float angle */
    std::unique_ptr<OpAndValues>
    RotateXIntLowering::createNewOpAndValues(qmem::RotateXIntOp op, qmem::RotateXIntOp::Adaptor adaptor,
                                             ConversionPatternRewriter &rewriter) const {
        // Use the integers coming from the "intermediate" operation
        auto newRotate = rewriter.create<netqasm::RotateXOp>(
                op.getLoc(), adaptor.getQ(),
                adaptor.getNValAttr(),
                adaptor.getExpValAttr()
                );
        return std::make_unique<OpAndValues>(newRotate.getOperation(), newRotate->getResults());
    }

    std::unique_ptr<OpAndValues>
    RotateYIntLowering::createNewOpAndValues(qmem::RotateYIntOp op, qmem::RotateYIntOp::Adaptor adaptor,
                                             ConversionPatternRewriter &rewriter) const {
        // Use the integers coming from the "intermediate" operation
        auto newRotate = rewriter.create<netqasm::RotateYOp>(
                op.getLoc(), adaptor.getQ(),
                adaptor.getNValAttr(),
                adaptor.getExpValAttr()
                );
        return std::make_unique<OpAndValues>(newRotate.getOperation(), newRotate->getResults());
    }

    std::unique_ptr<OpAndValues>
    RotateZIntLowering::createNewOpAndValues(qmem::RotateZIntOp op, qmem::RotateZIntOp::Adaptor adaptor,
                                             ConversionPatternRewriter &rewriter) const {
        // Use the integers coming from the "intermediate" operation
        auto newRotate = rewriter.create<netqasm::RotateZOp>(
                op.getLoc(), adaptor.getQ(),
                adaptor.getNValAttr(),
                adaptor.getExpValAttr()
                );
        return std::make_unique<OpAndValues>(newRotate.getOperation(), newRotate->getResults());
    }

    std::unique_ptr<OpAndValues>
    CRotXIntLowering::createNewOpAndValues(qmem::CrotXIntOp op, qmem::CrotXIntOp::Adaptor adaptor,
                                           ConversionPatternRewriter &rewriter) const {
        auto newCrotX = rewriter.create<netqasm::CrotXOp>(
                op.getLoc(), adaptor.getQin0(), adaptor.getQin1(),
                adaptor.getNValAttr(),
                adaptor.getExpValAttr()
                );
        return std::make_unique<OpAndValues>(newCrotX.getOperation(), newCrotX->getResults());
    }

    /* Lowering patterns for operations that should have been lowered by the "intra-dialect" lowering */
    std::unique_ptr<OpAndValues>
    RotateXLowering::createNewOpAndValues(qmem::RotateXOp op, qmem::RotateXOp::Adaptor adaptor,
                                          ConversionPatternRewriter &rewriter) const {
        // The angle is a float, we need to transform it *statically* to 2 integers
        auto f32Const = dyn_cast<arith::ConstantFloatOp>(adaptor.getAngle().getDefiningOp());
        assert(f32Const && "Expected constant to be constant float");

        // Convert the float angle to its 2-integers counterpart.
        const double floatAngleVal = f32Const.value().convertToDouble();
        std::vector<uint32_t> intsAngle = angle::transformDouble(floatAngleVal);
        // And use the results of the conversion as the arguments of the new rotate operation
        auto newRotate = rewriter.create<qmem::RotateXIntOp>(
                op.getLoc(), adaptor.getQ(),
                rewriter.getUI32IntegerAttr(intsAngle[0]),
                rewriter.getUI32IntegerAttr(intsAngle[1])
                );
        return std::make_unique<OpAndValues>(newRotate.getOperation(), newRotate->getResults());
    }

    std::unique_ptr<OpAndValues>
    RotateYLowering::createNewOpAndValues(qmem::RotateYOp op, qmem::RotateYOp::Adaptor adaptor,
                                          ConversionPatternRewriter &rewriter) const {
        // The angle is a float, we need to transform it *statically* to 2 integers
        auto f32Const = dyn_cast<arith::ConstantFloatOp>(adaptor.getAngle().getDefiningOp());
        assert(f32Const && "Expected constant to be constant float");

        // Convert the float angle to its 2-integers counterpart.
        const double floatAngleVal = f32Const.value().convertToDouble();
        std::vector<uint32_t> intsAngle = angle::transformDouble(floatAngleVal);
        // And use the results of the conversion as the arguments of the new rotate operation
        auto newRotate = rewriter.create<qmem::RotateYIntOp>(
                op.getLoc(), adaptor.getQ(),
                rewriter.getUI32IntegerAttr(intsAngle[0]),
                rewriter.getUI32IntegerAttr(intsAngle[1])
                );
        return std::make_unique<OpAndValues>(newRotate.getOperation(), newRotate->getResults());
    }

    std::unique_ptr<OpAndValues>
    RotateZLowering::createNewOpAndValues(qmem::RotateZOp op, qmem::RotateZOp::Adaptor adaptor,
                                          ConversionPatternRewriter &rewriter) const {
        // The angle is a float, we need to transform it *statically* to 2 integers
        auto f32Const = dyn_cast<arith::ConstantFloatOp>(adaptor.getAngle().getDefiningOp());
        assert(f32Const && "Expected constant to be constant float");

        // Convert the float angle to its 2-integers counterpart.
        const double floatAngleVal = f32Const.value().convertToDouble();
        std::vector<uint32_t> intsAngle = angle::transformDouble(floatAngleVal);
        // And use the results of the conversion as the arguments of the new rotate operation
        rewriter.getUI32IntegerAttr(intsAngle[0]);
        auto newRotate = rewriter.create<qmem::RotateZIntOp>(
                op.getLoc(), adaptor.getQ(),
                rewriter.getUI32IntegerAttr(intsAngle[0]),
                rewriter.getUI32IntegerAttr(intsAngle[1])
                );
        return std::make_unique<OpAndValues>(newRotate.getOperation(), newRotate->getResults());
    }

    std::unique_ptr<OpAndValues>
    CRotXLowering::createNewOpAndValues(qmem::CrotXOp op, qmem::CrotXOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const {
        // The angle is a float, we need to transform it *statically* to 2 integers
        auto f32Const = dyn_cast<arith::ConstantFloatOp>(adaptor.getAngle().getDefiningOp());
        assert(f32Const && "Expected constant to be constant float");

        // Convert the float angle to its 2-integers counterpart.
        const double floatAngleVal = f32Const.value().convertToDouble();
        std::vector<uint32_t> intsAngle = angle::transformDouble(floatAngleVal);
        // And use the results of the conversion as the arguments of the new rotate operation
        auto newRotate = rewriter.create<qmem::CrotXIntOp>(
                    op.getLoc(), adaptor.getQin0(), adaptor.getQin1(),
                    rewriter.getUI32IntegerAttr(intsAngle[0]),
                    rewriter.getUI32IntegerAttr(intsAngle[1])
                    );
        return std::make_unique<OpAndValues>(newRotate.getOperation(), newRotate->getResults());
    }
} // namespace qoala::conversion::mir