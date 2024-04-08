#ifndef QNET_TO_QMEM_PATTERNS
#define QNET_TO_QMEM_PATTERNS
#include "Dialect/QNet/QNet.h"
#include "Dialect/QMem/QMem.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/Support/Debug.h"

using namespace qoala::dialects;

namespace qoala::conversion {
    class QNetToQMemQubitTypeConverter : public TypeConverter {
      public:
        explicit QNetToQMemQubitTypeConverter(MLIRContext *ctx);
    };

    /**
     * Simple class template used to convert an operation of class SourceOp to
     * another one of class DestOp *in a one-to-one manner*.
     * @tparam SourceOp The source class of the source dialect to convert from
     * @tparam DestOp The destination class of the target dialect to covert to
     */
    template <typename SourceOp, typename DestOp>
    class SimpleOneToToneLoweringTemplate : public OpConversionPattern<SourceOp> {
      public:
        // Constructor simply matches the super class
        using OpConversionPattern<SourceOp>::OpConversionPattern;

        /**
         * Defines how to create an Operation on the destination dialect, using
         * the data offered by the adaptor class of the source operation.
         * Implementors of this signature are *highly* encouraged to use the
         * `create` methods of the rewriter object to create instances of the
         * operation on the target dialect. When using these methods, please be
         * mindful that you need to "match" one of the target operations' class
         * `build` methods with the arguments passed to the create:
         * rewriter.create<DestOp>(op.getLoc(), <args_to_match_a_build_signature>...)
         * In this sense, you might want to use the getters of the adaptor class
         * to get the data (and types) to match any of the generated builders of the
         * target operand class.
         * IMPORTANT: The returned operation *MUST* yield the same number of values
         * than the operation to replaced. If this is not the case, this class is not
         * suitable for the purpose.
         * @param op The operation on the source dialect to be replaced
         * @param adaptor The adaptor of th source operation to get data from
         * @param rewriter The rewriter object to easily create new operations on the
         *                 target dialect
         * @return An operation on the destination dialect
         */
        virtual DestOp createNewOp(SourceOp op, SourceOp::Adaptor adaptor,
                                   ConversionPatternRewriter &rewriter) const = 0;

        LogicalResult
        matchAndRewrite(SourceOp op, typename SourceOp::Adaptor adaptor,
                        ConversionPatternRewriter &rewriter) const override {
            llvm::dbgs() << "lowering operation : '" << op << "'\n";
            auto newOp = createNewOp(op, adaptor, rewriter);
            // We use the "replace op for op" method; This method check that the old and the new ops
            // yield the same number of SSA results
            rewriter.replaceOp(op, newOp);
            return success();
        }
    };

    /**
     * Class template used to convert an operation of class SourceOp to
     * another one of class DestOp, *where `DestOp` produces a different number
     * of values than `SourceOp`*.
     * @tparam SourceOp The source class of the source dialect to convert from
     * @tparam DestOp The destination class of the target dialect to covert to
     */
    template <typename SourceOp, typename DestOp>
    class DifferentValuesLoweringTemplate : public OpConversionPattern<SourceOp> {
    public:
        struct NewOpAndValues {
            DestOp newOp;
            ValueRange values;
        };
        // Constructor simply matches the super class
        using OpConversionPattern<SourceOp>::OpConversionPattern;

        /**
         * Defines how to create an Operation on the destination dialect, using
         * the data offered by the adaptor class of the source operation.
         * Implementors of this signature are *highly* encouraged to use the
         * `create` methods of the rewriter object to create instances of the
         * operation on the target dialect. When using these methods, please be
         * mindful that you need to "match" one of the target operations' class
         * `build` methods with the arguments passed to the create:
         * rewriter.create<DestOp>(op.getLoc(), <args_to_match_a_build_signature>...)
         * In this sense, you might want to use the getters of the adaptor class
         * to get the data (and types) to match any of the generated builders of the
         * target operand class.
         * IMPORTANT: The returned operation *MUST* yield the same number of values
         * than the operation to replaced. If this is not the case, this class is not
         * suitable for the purpose.
         * @param op The operation on the source dialect to be replaced
         * @param adaptor The adaptor of th source operation to get data from
         * @param rewriter The rewriter object to easily create new operations on the
         *                 target dialect
         * @return An operation on the destination dialect
         */
        virtual NewOpAndValues createNewOpAndValues(SourceOp op, SourceOp::Adaptor adaptor,
                                                    ConversionPatternRewriter &rewriter) const = 0;

        LogicalResult
        matchAndRewrite(SourceOp op, typename SourceOp::Adaptor adaptor,
                        ConversionPatternRewriter &rewriter) const override {
            llvm::dbgs() << "lowering operation : '" << op << "'\n";
            NewOpAndValues newOp = createNewOpAndValues(op, adaptor, rewriter);
            // We use the "replace op for values" method; This method check that the old op
            // yield the same number of SSA results as the given values
            rewriter.replaceOp(op, newOp.values);
            return success();
        }
    };

    class FuncOpLowering : public SimpleOneToToneLoweringTemplate<qnet::FuncOp, qmem::FuncOp> {
    public:
        // Constructor simply refers to the parent
        using SimpleOneToToneLoweringTemplate::SimpleOneToToneLoweringTemplate;
        qmem::FuncOp createNewOp(qnet::FuncOp op, qnet::FuncOp::Adaptor adaptor,
                                 ConversionPatternRewriter &rewriter) const override;
    };

    class ReturnOpLowering : public SimpleOneToToneLoweringTemplate<qnet::ReturnOp, qmem::ReturnOp> {
    public:
        // Constructor simply refers to the parent
        using SimpleOneToToneLoweringTemplate::SimpleOneToToneLoweringTemplate;
        qmem::ReturnOp createNewOp(qnet::ReturnOp op, qnet::ReturnOp::Adaptor adaptor,
                                   ConversionPatternRewriter &rewriter) const override;
    };

    /* Remote operations can be mapped with a simple one-to-one operation */
    class RemoteOpLowering : public SimpleOneToToneLoweringTemplate<qnet::RemoteOp, qmem::RemoteOp> {
    public:
        // Constructor simply refers to the parent
        using SimpleOneToToneLoweringTemplate::SimpleOneToToneLoweringTemplate;
        qmem::RemoteOp createNewOp(qnet::RemoteOp op, qnet::RemoteOp::Adaptor adaptor,
                                   ConversionPatternRewriter &rewriter) const override;
    };
    class SendIntsOpLowering : public SimpleOneToToneLoweringTemplate<qnet::SendIntsOp, qmem::SendIntsOp> {
    public:
        // Constructor simply refers to the parent
        using SimpleOneToToneLoweringTemplate::SimpleOneToToneLoweringTemplate;
        qmem::SendIntsOp createNewOp(qnet::SendIntsOp op, qnet::SendIntsOp::Adaptor adaptor,
                                     ConversionPatternRewriter &rewriter) const override;
    };
    class RecvIntsOpLowering : public SimpleOneToToneLoweringTemplate<qnet::RecvIntsOp, qmem::RecvIntsOp> {
    public:
        // Constructor simply refers to the parent
        using SimpleOneToToneLoweringTemplate::SimpleOneToToneLoweringTemplate;
        qmem::RecvIntsOp createNewOp(qnet::RecvIntsOp op, qnet::RecvIntsOp::Adaptor adaptor,
                                     ConversionPatternRewriter &rewriter) const override;
    };
    class SendFloatsOpLowering : public SimpleOneToToneLoweringTemplate<qnet::SendFloatsOp, qmem::SendFloatsOp> {
    public:
        // Constructor simply refers to the parent
        using SimpleOneToToneLoweringTemplate::SimpleOneToToneLoweringTemplate;
        qmem::SendFloatsOp createNewOp(qnet::SendFloatsOp op, qnet::SendFloatsOp::Adaptor adaptor,
                                       ConversionPatternRewriter &rewriter) const override;
    };
    class RecvFloatsOpLowering : public SimpleOneToToneLoweringTemplate<qnet::RecvFloatsOp, qmem::RecvFloatsOp> {
    public:
        // Constructor simply refers to the parent
        using SimpleOneToToneLoweringTemplate::SimpleOneToToneLoweringTemplate;
        qmem::RecvFloatsOp createNewOp(qnet::RecvFloatsOp op, qnet::RecvFloatsOp::Adaptor adaptor,
                                       ConversionPatternRewriter &rewriter) const override;
    };
    class NewQubitLowering : public SimpleOneToToneLoweringTemplate<qnet::NewQubitOp, qmem::QAllocOp> {
    public:
        // Constructor simply refers to the parent
        using SimpleOneToToneLoweringTemplate::SimpleOneToToneLoweringTemplate;
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

    // TODO - instantiate the template to map operations from one dialect to the other

} // namespace qoala::conversion

#endif // QNET_TO_QMEM_PATTERNS
