#ifndef QOALAMIR_TO_QOALALIR_PATTERNS
#define QOALAMIR_TO_QOALALIR_PATTERNS
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "Conversion/Helpers/Helpers.h"
#include "Dialect/QMem/QMem.h"
#include "Dialect/QRemote/QRemote.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Dialect/NetQASM/NetQASM.h"

namespace qoala::conversion::mir {
    /* Lowering for operations define the main function or are inside it - Will map to QoalaHost dialect */
    class RemoteOpLowering: public helpers::OpLoweringTemplate<dialects::qmem::RemoteOp, dialects::qremote::RemoteOp> {
    public:
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qmem::RemoteOp op, dialects::qmem::RemoteOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class FuncOpLowering : public helpers::OpLoweringTemplate<dialects::qmem::FuncOp, dialects::qoalahost::MainFuncOp> {
    public:
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qmem::FuncOp op, dialects::qmem::FuncOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class ReturnOpLowering : public helpers::OpLoweringTemplate<dialects::qmem::ReturnOp, dialects::qoalahost::ReturnOp> {
    public:
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qmem::ReturnOp op, dialects::qmem::ReturnOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class CallOpLowering : public helpers::OpLoweringTemplate<func::CallOp, dialects::qoalahost::CallOp> {
    public:
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(func::CallOp op, func::CallOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class RecvIntsOpLowering : public helpers::OpLoweringTemplate<dialects::qmem::RecvIntsOp, dialects::qoalahost::RecvIntsOp> {
    public:
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qmem::RecvIntsOp op, dialects::qmem::RecvIntsOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class RecvFloatsOpLowering : public helpers::OpLoweringTemplate<dialects::qmem::RecvFloatsOp, dialects::qoalahost::RecvFloatsOp> {
    public:
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qmem::RecvFloatsOp op, dialects::qmem::RecvFloatsOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class SendIntsOpLowering : public helpers::OpLoweringTemplate<dialects::qmem::SendIntsOp, dialects::qoalahost::SendIntsOp> {
    public:
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qmem::SendIntsOp op, dialects::qmem::SendIntsOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class SendFloatsOpLowering : public helpers::OpLoweringTemplate<dialects::qmem::SendFloatsOp, dialects::qoalahost::SendFloatsOp> {
    public:
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qmem::SendFloatsOp op, dialects::qmem::SendFloatsOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };

    /* Lowering for operations that define or are inside local_routine or request_routine - Will map to NetQASM dialect */
    class MeasureOpLowering : public helpers::OpLoweringTemplate<dialects::qmem::MeasureOp, dialects::netqasm::MeasureOp> {
    public:
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qmem::MeasureOp op, dialects::qmem::MeasureOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class EprsOpLowering : public helpers::OpLoweringTemplate<dialects::qmem::EprsOp, dialects::netqasm::EprsOp> {
    public:
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qmem::EprsOp op, dialects::qmem::EprsOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class EprsMeasureOpLowering : public helpers::OpLoweringTemplate<dialects::qmem::EprsMeasureOp, dialects::netqasm::EprsMeasureOp> {
    public:
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qmem::EprsMeasureOp op, dialects::qmem::EprsMeasureOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class NetQASMFunctionLowering :
            public helpers::OpLoweringTemplate<func::FuncOp, dialects::netqasm::LocalRoutineOp, dialects::netqasm::RequestRoutineOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(func::FuncOp op, func::FuncOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class NetQASMReturnOpLowering : public helpers::OpLoweringTemplate<func::ReturnOp, dialects::netqasm::ReturnOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(func::ReturnOp op, func::ReturnOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class QAllocLowering : public helpers::OpLoweringTemplate<dialects::qmem::QAllocOp, dialects::netqasm::QAllocOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qmem::QAllocOp op, dialects::qmem::QAllocOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class QInitLowering : public helpers::OpLoweringTemplate<dialects::qmem::InitOp, dialects::netqasm::QInitOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qmem::InitOp op, dialects::qmem::InitOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class HadamardLowering : public helpers::OpLoweringTemplate<dialects::qmem::HadamardOp, dialects::netqasm::HadamardOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qmem::HadamardOp op, dialects::qmem::HadamardOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class CNotLowering : public helpers::OpLoweringTemplate<dialects::qmem::CnotOp, dialects::netqasm::CnotOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qmem::CnotOp op, dialects::qmem::CnotOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class CzLowering : public helpers::OpLoweringTemplate<dialects::qmem::CzOp, dialects::netqasm::CzOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qmem::CzOp op, dialects::qmem::CzOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    /* Lowering for "intermediate" operations that use the angle_num and angle_denom integers
     * instead of float angle
     */
    class RotateXIntLowering : public helpers::OpLoweringTemplate<dialects::qmem::RotateXIntOp, dialects::netqasm::RotateXOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qmem::RotateXIntOp op, dialects::qmem::RotateXIntOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class RotateYIntLowering : public helpers::OpLoweringTemplate<dialects::qmem::RotateYIntOp, dialects::netqasm::RotateYOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qmem::RotateYIntOp op, dialects::qmem::RotateYIntOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class RotateZIntLowering : public helpers::OpLoweringTemplate<dialects::qmem::RotateZIntOp, dialects::netqasm::RotateZOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qmem::RotateZIntOp op, dialects::qmem::RotateZIntOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class CRotXIntLowering : public helpers::OpLoweringTemplate<dialects::qmem::CrotXIntOp, dialects::netqasm::CrotXOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qmem::CrotXIntOp op, dialects::qmem::CrotXIntOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };

    /* "Intra-dialect" lowering patterns for operations that use f32 argument, and need to be
     * converted to their i32 counterpart
     */
    class RotateXLowering : public helpers::OpLoweringTemplate<dialects::qmem::RotateXOp, dialects::qmem::RotateXIntOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qmem::RotateXOp op, dialects::qmem::RotateXOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class RotateYLowering : public helpers::OpLoweringTemplate<dialects::qmem::RotateYOp, dialects::qmem::RotateYIntOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qmem::RotateYOp op, dialects::qmem::RotateYOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class RotateZLowering : public helpers::OpLoweringTemplate<dialects::qmem::RotateZOp, dialects::qmem::RotateZIntOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qmem::RotateZOp op, dialects::qmem::RotateZOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class CRotXLowering : public helpers::OpLoweringTemplate<dialects::qmem::CrotXOp, dialects::qmem::CrotXIntOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qmem::CrotXOp op, dialects::qmem::CrotXOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
} // namespace qoala::conversion::mir

#endif // QOALAMIR_TO_QOALALIR_PATTERNS
