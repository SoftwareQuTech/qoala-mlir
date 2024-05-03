#ifndef QOALAMIR_TO_QOALALIR_PATTERNS
#define QOALAMIR_TO_QOALALIR_PATTERNS
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "Conversion/Helpers/Helpers.h"
#include "Dialect/QMem/QMem.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Dialect/NetQASM/NetQASM.h"

using namespace qoala::dialects;
using namespace qoala::helpers;

namespace qoala::conversion::mir {

    func::CallOp insertCallAngleTransform(Operation *operation,
                                          ConversionPatternRewriter &rewriter,
                                          Value angle);

    /* Lowering for operations define the main function or are inside it - Will map to QoalaHost dialect */
    class RemoteOpLowering: public OpLoweringTemplate<qmem::RemoteOp, qoalahost::RemoteOp> {
    public:
        using OpLoweringTemplate::OpLoweringTemplate;
        ValueRange createNewOpAndValues(qmem::RemoteOp op, qmem::RemoteOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class FuncOpLowering : public OpLoweringTemplate<qmem::FuncOp, qoalahost::MainFuncOp> {
    public:
        using OpLoweringTemplate::OpLoweringTemplate;
        ValueRange createNewOpAndValues(qmem::FuncOp op, qmem::FuncOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class ReturnOpLowering : public OpLoweringTemplate<qmem::ReturnOp, qoalahost::ReturnOp> {
    public:
        using OpLoweringTemplate::OpLoweringTemplate;
        ValueRange createNewOpAndValues(qmem::ReturnOp op, qmem::ReturnOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class CallOpLowering : public OpLoweringTemplate<func::CallOp, qoalahost::CallOp> {
    public:
        using OpLoweringTemplate::OpLoweringTemplate;
        ValueRange createNewOpAndValues(func::CallOp op, func::CallOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class RecvIntsOpLowering : public OpLoweringTemplate<qmem::RecvIntsOp, qoalahost::RecvIntsOp> {
    public:
        using OpLoweringTemplate::OpLoweringTemplate;
        ValueRange createNewOpAndValues(qmem::RecvIntsOp op, qmem::RecvIntsOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class RecvFloatsOpLowering : public OpLoweringTemplate<qmem::RecvFloatsOp, qoalahost::RecvFloatsOp> {
    public:
        using OpLoweringTemplate::OpLoweringTemplate;
        ValueRange createNewOpAndValues(qmem::RecvFloatsOp op, qmem::RecvFloatsOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };

    /* Lowering for operations that define or are inside local_routine or request_routine - Will map to NetQASM dialect */
    class MeasureOpLowering : public OpLoweringTemplate<qmem::MeasureOp, netqasm::MeasureOp> {
    public:
        using OpLoweringTemplate::OpLoweringTemplate;
        ValueRange createNewOpAndValues(qmem::MeasureOp op, qmem::MeasureOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class EprsOpLowering : public OpLoweringTemplate<qmem::EprsOp, netqasm::EprsOp> {
    public:
        using OpLoweringTemplate::OpLoweringTemplate;
        ValueRange createNewOpAndValues(qmem::EprsOp op, qmem::EprsOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class EprsMeasureOpLowering : public OpLoweringTemplate<qmem::EprsMeasureOp, netqasm::EprsMeasureOp> {
    public:
        using OpLoweringTemplate::OpLoweringTemplate;
        ValueRange createNewOpAndValues(qmem::EprsMeasureOp op, qmem::EprsMeasureOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class NetQASMFunctionLowering :
            public OpLoweringTemplate<func::FuncOp,std::variant<netqasm::LocalRoutineOp, netqasm::RequestRoutineOp>> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        ValueRange createNewOpAndValues(func::FuncOp op, func::FuncOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class NetQASMReturnOpLowering : public OpLoweringTemplate<func::ReturnOp, netqasm::ReturnOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        ValueRange createNewOpAndValues(func::ReturnOp op, func::ReturnOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class QAllocLowering : public OpLoweringTemplate<qmem::QAllocOp, netqasm::QAllocOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        ValueRange createNewOpAndValues(qmem::QAllocOp op, qmem::QAllocOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class QInitLowering : public OpLoweringTemplate<qmem::InitOp, netqasm::QInitOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        ValueRange createNewOpAndValues(qmem::InitOp op, qmem::InitOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class HadamardLowering : public OpLoweringTemplate<qmem::HadamardOp, netqasm::HadamardOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        ValueRange createNewOpAndValues(qmem::HadamardOp op, qmem::HadamardOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class CNotLowering : public OpLoweringTemplate<qmem::CnotOp, netqasm::CnotOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        ValueRange createNewOpAndValues(qmem::CnotOp op, qmem::CnotOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class CzLowering : public OpLoweringTemplate<qmem::CzOp, netqasm::CzOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        ValueRange createNewOpAndValues(qmem::CzOp op, qmem::CzOp::Adaptor adaptor,
                                  ConversionPatternRewriter &rewriter) const override;
    };
    /* Lowering for "intermediate" operations that use the angle_num and angle_denom integers
     * instead of float angle
     */
    class RotateXIntLowering : public OpLoweringTemplate<qmem::RotateXIntOp, netqasm::RotateXOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        ValueRange createNewOpAndValues(qmem::RotateXIntOp op, qmem::RotateXIntOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class RotateYIntLowering : public OpLoweringTemplate<qmem::RotateYIntOp, netqasm::RotateYOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        ValueRange createNewOpAndValues(qmem::RotateYIntOp op, qmem::RotateYIntOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class RotateZIntLowering : public OpLoweringTemplate<qmem::RotateZIntOp, netqasm::RotateZOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        ValueRange createNewOpAndValues(qmem::RotateZIntOp op, qmem::RotateZIntOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class CRotXIntLowering : public OpLoweringTemplate<qmem::CrotXIntOp, netqasm::CrotXOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        ValueRange createNewOpAndValues(qmem::CrotXIntOp op, qmem::CrotXIntOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };

    /* "Intra-dialect" lowering patterns for operations that use f32 argument, and need to be
     * converted to their i32 counterpart
     */
    class RotateXLowering : public OpLoweringTemplate<qmem::RotateXOp, qmem::RotateXIntOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        ValueRange createNewOpAndValues(qmem::RotateXOp op, qmem::RotateXOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class RotateYLowering : public OpLoweringTemplate<qmem::RotateYOp, qmem::RotateYIntOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        ValueRange createNewOpAndValues(qmem::RotateYOp op, qmem::RotateYOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class RotateZLowering : public OpLoweringTemplate<qmem::RotateZOp, qmem::RotateZIntOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        ValueRange createNewOpAndValues(qmem::RotateZOp op, qmem::RotateZOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class CRotXLowering : public OpLoweringTemplate<qmem::CrotXOp, qmem::CrotXIntOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        ValueRange createNewOpAndValues(qmem::CrotXOp op, qmem::CrotXOp::Adaptor adaptor,
                                     ConversionPatternRewriter &rewriter) const override;
    };
} // namespace qoala::conversion::mir

#endif // QOALAMIR_TO_QOALALIR_PATTERNS
