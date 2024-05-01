#ifndef QNET_TO_QMEM_PATTERNS
#define QNET_TO_QMEM_PATTERNS
#include "Dialect/QNet/QNet.h"
#include "Dialect/QMem/QMem.h"
#include "mlir/Transforms/DialectConversion.h"
#include "Conversion/Helpers/Helpers.h"

using namespace qoala::dialects;
using namespace qoala::helpers;

namespace qoala::conversion {
    class QoalaHIRToQoalaMIRTypeConverter : public TypeConverter {
      public:
        explicit QoalaHIRToQoalaMIRTypeConverter(MLIRContext *ctx);
    };

    class FuncOpLowering : public SimpleOneToToOneLoweringTemplate<qnet::FuncOp, qmem::FuncOp> {
    public:
        // Constructor simply refers to the parent
        using SimpleOneToToOneLoweringTemplate::SimpleOneToToOneLoweringTemplate;
        qmem::FuncOp createNewOp(qnet::FuncOp op, qnet::FuncOp::Adaptor adaptor,
                                 ConversionPatternRewriter &rewriter) const override;
    };

    class ReturnOpLowering : public SimpleOneToToOneLoweringTemplate<qnet::ReturnOp, qmem::ReturnOp> {
    public:
        // Constructor simply refers to the parent
        using SimpleOneToToOneLoweringTemplate::SimpleOneToToOneLoweringTemplate;
        qmem::ReturnOp createNewOp(qnet::ReturnOp op, qnet::ReturnOp::Adaptor adaptor,
                                   ConversionPatternRewriter &rewriter) const override;
    };

    /* Operations used to create entanglement */
    class EprsOpLowering : public DifferentValuesLoweringTemplate<qnet::EprsOp, qmem::EprsOp> {
    public:
        // Constructor simply matches the super class
        using DifferentValuesLoweringTemplate::DifferentValuesLoweringTemplate;

        NewOpAndValues createNewOpAndValues(qnet::EprsOp op, qnet::EprsOp::Adaptor adaptor,
                                            ConversionPatternRewriter &rewriter) const override;
    };

    class EprsMeasureOpLowering : public SimpleOneToToOneLoweringTemplate<qnet::EprsMeasureOp, qmem::EprsMeasureOp> {
    public:
        // Constructor simply refers to the parent
        using SimpleOneToToOneLoweringTemplate::SimpleOneToToOneLoweringTemplate;
        qmem::EprsMeasureOp createNewOp(qnet::EprsMeasureOp op, qnet::EprsMeasureOp::Adaptor adaptor,
                                        ConversionPatternRewriter &rewriter) const override;
    };

    /* Remote operations can be mapped with a simple one-to-one operation */
    class RemoteOpLowering : public SimpleOneToToOneLoweringTemplate<qnet::RemoteOp, qmem::RemoteOp> {
    public:
        // Constructor simply refers to the parent
        using SimpleOneToToOneLoweringTemplate::SimpleOneToToOneLoweringTemplate;
        qmem::RemoteOp createNewOp(qnet::RemoteOp op, qnet::RemoteOp::Adaptor adaptor,
                                   ConversionPatternRewriter &rewriter) const override;
    };
    class SendIntsOpLowering : public SimpleOneToToOneLoweringTemplate<qnet::SendIntsOp, qmem::SendIntsOp> {
    public:
        // Constructor simply refers to the parent
        using SimpleOneToToOneLoweringTemplate::SimpleOneToToOneLoweringTemplate;
        qmem::SendIntsOp createNewOp(qnet::SendIntsOp op, qnet::SendIntsOp::Adaptor adaptor,
                                     ConversionPatternRewriter &rewriter) const override;
    };
    class RecvIntsOpLowering : public SimpleOneToToOneLoweringTemplate<qnet::RecvIntsOp, qmem::RecvIntsOp> {
    public:
        // Constructor simply refers to the parent
        using SimpleOneToToOneLoweringTemplate::SimpleOneToToOneLoweringTemplate;
        qmem::RecvIntsOp createNewOp(qnet::RecvIntsOp op, qnet::RecvIntsOp::Adaptor adaptor,
                                     ConversionPatternRewriter &rewriter) const override;
    };
    class SendFloatsOpLowering : public SimpleOneToToOneLoweringTemplate<qnet::SendFloatsOp, qmem::SendFloatsOp> {
    public:
        // Constructor simply refers to the parent
        using SimpleOneToToOneLoweringTemplate::SimpleOneToToOneLoweringTemplate;
        qmem::SendFloatsOp createNewOp(qnet::SendFloatsOp op, qnet::SendFloatsOp::Adaptor adaptor,
                                       ConversionPatternRewriter &rewriter) const override;
    };
    class RecvFloatsOpLowering : public SimpleOneToToOneLoweringTemplate<qnet::RecvFloatsOp, qmem::RecvFloatsOp> {
    public:
        // Constructor simply refers to the parent
        using SimpleOneToToOneLoweringTemplate::SimpleOneToToOneLoweringTemplate;
        qmem::RecvFloatsOp createNewOp(qnet::RecvFloatsOp op, qnet::RecvFloatsOp::Adaptor adaptor,
                                       ConversionPatternRewriter &rewriter) const override;
    };
    class NewQubitLowering : public SimpleOneToToOneLoweringTemplate<qnet::NewQubitOp, qmem::QAllocOp> {
    public:
        // Constructor simply refers to the parent
        using SimpleOneToToOneLoweringTemplate::SimpleOneToToOneLoweringTemplate;
        qmem::QAllocOp createNewOp(qnet::NewQubitOp op, qnet::NewQubitOp::Adaptor adaptor,
                                   ConversionPatternRewriter &rewriter) const override;
    };
    class RotateXLowering : public DifferentValuesLoweringTemplate<qnet::RotXOp, qmem::RotateXOp> {
    public:
        // Constructor simply matches the super class
        using DifferentValuesLoweringTemplate::DifferentValuesLoweringTemplate;

        NewOpAndValues createNewOpAndValues(qnet::RotXOp op, qnet::RotXOp::Adaptor adaptor,
                                            ConversionPatternRewriter &rewriter) const override;
    };
    class RotateYLowering : public DifferentValuesLoweringTemplate<qnet::RotYOp, qmem::RotateYOp> {
    public:
        // Constructor simply matches the super class
        using DifferentValuesLoweringTemplate::DifferentValuesLoweringTemplate;

        NewOpAndValues createNewOpAndValues(qnet::RotYOp op, qnet::RotYOp::Adaptor adaptor,
                                            ConversionPatternRewriter &rewriter) const override;
    };
    class RotateZLowering : public DifferentValuesLoweringTemplate<qnet::RotZOp, qmem::RotateZOp> {
    public:
        // Constructor simply matches the super class
        using DifferentValuesLoweringTemplate::DifferentValuesLoweringTemplate;

        NewOpAndValues createNewOpAndValues(qnet::RotZOp op, qnet::RotZOp::Adaptor adaptor,
                                            ConversionPatternRewriter &rewriter) const override;
    };
    class MeasureLowering : public DifferentValuesLoweringTemplate<qnet::MeasureOp, qmem::MeasureOp> {
    public:
        // Constructor simply matches the super class
        using DifferentValuesLoweringTemplate::DifferentValuesLoweringTemplate;

        NewOpAndValues createNewOpAndValues(qnet::MeasureOp op, qnet::MeasureOp::Adaptor adaptor,
                                            ConversionPatternRewriter &rewriter) const override;
    };
    class HadamardLowering : public DifferentValuesLoweringTemplate<qnet::HadamardOp, qmem::HadamardOp> {
    public:
        // Constructor simply matches the super class
        using DifferentValuesLoweringTemplate::DifferentValuesLoweringTemplate;

        NewOpAndValues createNewOpAndValues(qnet::HadamardOp op, qnet::HadamardOp::Adaptor adaptor,
                                            ConversionPatternRewriter &rewriter) const override;
    };
    class CNotLowering : public DifferentValuesLoweringTemplate<qnet::CnotOp, qmem::CnotOp> {
    public:
        // Constructor simply matches the super class
        using DifferentValuesLoweringTemplate::DifferentValuesLoweringTemplate;

        NewOpAndValues createNewOpAndValues(qnet::CnotOp op, qnet::CnotOp::Adaptor adaptor,
                                            ConversionPatternRewriter &rewriter) const override;
    };
    class CzLowering : public DifferentValuesLoweringTemplate<qnet::CzOp, qmem::CzOp> {
    public:
        // Constructor simply matches the super class
        using DifferentValuesLoweringTemplate::DifferentValuesLoweringTemplate;

        NewOpAndValues createNewOpAndValues(qnet::CzOp op, qnet::CzOp::Adaptor adaptor,
                                            ConversionPatternRewriter &rewriter) const override;
    };
    class CRotXLowering : public DifferentValuesLoweringTemplate<qnet::CrotXOp, qmem::CrotXOp> {
    public:
        // Constructor simply matches the super class
        using DifferentValuesLoweringTemplate::DifferentValuesLoweringTemplate;

        NewOpAndValues createNewOpAndValues(qnet::CrotXOp op, qnet::CrotXOp::Adaptor adaptor,
                                            ConversionPatternRewriter &rewriter) const override;
    };

} // namespace qoala::conversion

#endif // QNET_TO_QMEM_PATTERNS
