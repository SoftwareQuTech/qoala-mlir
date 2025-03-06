#ifndef QNET_TO_QMEM_PATTERNS
#define QNET_TO_QMEM_PATTERNS
#include "Dialect/QNet/QNet.h"
#include "Dialect/QMem/QMem.h"
#include "mlir/Transforms/DialectConversion.h"
#include "Conversion/Helpers/Helpers.h"

namespace qoala::conversion::hir {
    class QoalaHIRToQoalaMIRTypeConverter : public TypeConverter {
      public:
        explicit QoalaHIRToQoalaMIRTypeConverter(MLIRContext *ctx);
    };

    class FuncOpLowering : public helpers::OpLoweringTemplate<dialects::qnet::FuncOp, dialects::qmem::FuncOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::FuncOp op, dialects::qnet::FuncOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };

    class ReturnOpLowering : public helpers::OpLoweringTemplate<dialects::qnet::ReturnOp, dialects::qmem::ReturnOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::ReturnOp op, dialects::qnet::ReturnOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };

    /* Operations used to create entanglement */
    class EprsOpLowering : public helpers::OpLoweringTemplate<dialects::qnet::EprsOp, dialects::qmem::EprsOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::EprsOp op, dialects::qnet::EprsOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };

    class EprsMeasureOpLowering : public helpers::OpLoweringTemplate<dialects::qnet::EprsMeasureOp, dialects::qmem::EprsMeasureOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::EprsMeasureOp op, dialects::qnet::EprsMeasureOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };

    /* Remote operations can be mapped with a simple one-to-one operation */
    class RemoteOpLowering : public helpers::OpLoweringTemplate<dialects::qnet::RemoteOp, dialects::qmem::RemoteOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::RemoteOp op, dialects::qnet::RemoteOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class SendIntsOpLowering : public helpers::OpLoweringTemplate<dialects::qnet::SendIntsOp, dialects::qmem::SendIntsOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::SendIntsOp op, dialects::qnet::SendIntsOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class RecvIntsOpLowering : public helpers::OpLoweringTemplate<dialects::qnet::RecvIntsOp, dialects::qmem::RecvIntsOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::RecvIntsOp op, dialects::qnet::RecvIntsOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class SendFloatsOpLowering : public helpers::OpLoweringTemplate<dialects::qnet::SendFloatsOp, dialects::qmem::SendFloatsOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::SendFloatsOp op, dialects::qnet::SendFloatsOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class RecvFloatsOpLowering : public helpers::OpLoweringTemplate<dialects::qnet::RecvFloatsOp, dialects::qmem::RecvFloatsOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::RecvFloatsOp op, dialects::qnet::RecvFloatsOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class NewQubitLowering : public helpers::OpLoweringTemplate<dialects::qnet::NewQubitOp, dialects::qmem::QAllocOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::NewQubitOp op, dialects::qnet::NewQubitOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class RotateXLowering : public helpers::OpLoweringTemplate<dialects::qnet::RotXOp, dialects::qmem::RotateXOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::RotXOp op, dialects::qnet::RotXOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class RotateYLowering : public helpers::OpLoweringTemplate<dialects::qnet::RotYOp, dialects::qmem::RotateYOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::RotYOp op, dialects::qnet::RotYOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class RotateZLowering : public helpers::OpLoweringTemplate<dialects::qnet::RotZOp, dialects::qmem::RotateZOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::RotZOp op, dialects::qnet::RotZOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class MeasureLowering : public helpers::OpLoweringTemplate<dialects::qnet::MeasureOp, dialects::qmem::MeasureOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::MeasureOp op, dialects::qnet::MeasureOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class HadamardLowering : public helpers::OpLoweringTemplate<dialects::qnet::HadamardOp, dialects::qmem::HadamardOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::HadamardOp op, dialects::qnet::HadamardOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class CNotLowering : public helpers::OpLoweringTemplate<dialects::qnet::CnotOp, dialects::qmem::CnotOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::CnotOp op, dialects::qnet::CnotOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class CzLowering : public helpers::OpLoweringTemplate<dialects::qnet::CzOp, dialects::qmem::CzOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::CzOp op, dialects::qnet::CzOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };
    class CRotXLowering : public helpers::OpLoweringTemplate<dialects::qnet::CrotXOp, dialects::qmem::CrotXOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        std::unique_ptr<helpers::OpAndValues>
        createNewOpAndValues(dialects::qnet::CrotXOp op, dialects::qnet::CrotXOp::Adaptor adaptor,
                             ConversionPatternRewriter &rewriter) const override;
    };

} // namespace qoala::conversion::hir

#endif // QNET_TO_QMEM_PATTERNS
