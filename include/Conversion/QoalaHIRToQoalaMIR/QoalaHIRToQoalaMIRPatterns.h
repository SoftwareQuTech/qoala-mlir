#ifndef QNET_TO_QMEM_PATTERNS
#define QNET_TO_QMEM_PATTERNS
#include "Dialect/QNet/QNet.h"
#include "Dialect/QMem/QMem.h"
#include "mlir/Transforms/DialectConversion.h"
#include "Conversion/Helpers/Helpers.h"

using namespace qoala::dialects;
using namespace qoala::helpers;

namespace qoala::conversion::hir {
    class QoalaHIRToQoalaMIRTypeConverter : public TypeConverter {
      public:
        explicit QoalaHIRToQoalaMIRTypeConverter(MLIRContext *ctx);
    };

    class FuncOpLowering : public OpLoweringTemplate<qnet::FuncOp, qmem::FuncOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        ValueRange createNewOpAndValues(qnet::FuncOp op, qnet::FuncOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };

    class ReturnOpLowering : public OpLoweringTemplate<qnet::ReturnOp, qmem::ReturnOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        ValueRange createNewOpAndValues(qnet::ReturnOp op, qnet::ReturnOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };

    /* Operations used to create entanglement */
    class EprsOpLowering : public OpLoweringTemplate<qnet::EprsOp, qmem::EprsOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        ValueRange createNewOpAndValues(qnet::EprsOp op, qnet::EprsOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };

    class EprsMeasureOpLowering : public OpLoweringTemplate<qnet::EprsMeasureOp, qmem::EprsMeasureOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        ValueRange createNewOpAndValues(qnet::EprsMeasureOp op, qnet::EprsMeasureOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };

    /* Remote operations can be mapped with a simple one-to-one operation */
    class RemoteOpLowering : public OpLoweringTemplate<qnet::RemoteOp, qmem::RemoteOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        ValueRange createNewOpAndValues(qnet::RemoteOp op, qnet::RemoteOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class SendIntsOpLowering : public OpLoweringTemplate<qnet::SendIntsOp, qmem::SendIntsOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        ValueRange createNewOpAndValues(qnet::SendIntsOp op, qnet::SendIntsOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class RecvIntsOpLowering : public OpLoweringTemplate<qnet::RecvIntsOp, qmem::RecvIntsOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        ValueRange createNewOpAndValues(qnet::RecvIntsOp op, qnet::RecvIntsOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class SendFloatsOpLowering : public OpLoweringTemplate<qnet::SendFloatsOp, qmem::SendFloatsOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        ValueRange createNewOpAndValues(qnet::SendFloatsOp op, qnet::SendFloatsOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class RecvFloatsOpLowering : public OpLoweringTemplate<qnet::RecvFloatsOp, qmem::RecvFloatsOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        ValueRange createNewOpAndValues(qnet::RecvFloatsOp op, qnet::RecvFloatsOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class NewQubitLowering : public OpLoweringTemplate<qnet::NewQubitOp, qmem::QAllocOp> {
    public:
        // Constructor simply refers to the parent
        using OpLoweringTemplate::OpLoweringTemplate;
        ValueRange createNewOpAndValues(qnet::NewQubitOp op, qnet::NewQubitOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class RotateXLowering : public OpLoweringTemplate<qnet::RotXOp, qmem::RotateXOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        ValueRange createNewOpAndValues(qnet::RotXOp op, qnet::RotXOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class RotateYLowering : public OpLoweringTemplate<qnet::RotYOp, qmem::RotateYOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        ValueRange createNewOpAndValues(qnet::RotYOp op, qnet::RotYOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class RotateZLowering : public OpLoweringTemplate<qnet::RotZOp, qmem::RotateZOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        ValueRange createNewOpAndValues(qnet::RotZOp op, qnet::RotZOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class MeasureLowering : public OpLoweringTemplate<qnet::MeasureOp, qmem::MeasureOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        ValueRange createNewOpAndValues(qnet::MeasureOp op, qnet::MeasureOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class HadamardLowering : public OpLoweringTemplate<qnet::HadamardOp, qmem::HadamardOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        ValueRange createNewOpAndValues(qnet::HadamardOp op, qnet::HadamardOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class CNotLowering : public OpLoweringTemplate<qnet::CnotOp, qmem::CnotOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        ValueRange createNewOpAndValues(qnet::CnotOp op, qnet::CnotOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class CzLowering : public OpLoweringTemplate<qnet::CzOp, qmem::CzOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        ValueRange createNewOpAndValues(qnet::CzOp op, qnet::CzOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };
    class CRotXLowering : public OpLoweringTemplate<qnet::CrotXOp, qmem::CrotXOp> {
    public:
        // Constructor simply matches the super class
        using OpLoweringTemplate::OpLoweringTemplate;

        ValueRange createNewOpAndValues(qnet::CrotXOp op, qnet::CrotXOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };

} // namespace qoala::conversion::hir

#endif // QNET_TO_QMEM_PATTERNS
