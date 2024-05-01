#ifndef QOALA_MLIR_HELPERS_H
#define QOALA_MLIR_HELPERS_H

#include "mlir/Transforms/DialectConversion.h"

using namespace mlir;

namespace qoala::helpers {
    /* Helper template class used to create simple operation rewriter classes */

    /**
     * Simple class template used to convert an operation of class SourceOp to
     * another one of class DestOp *in a one-to-one manner*.
     * @tparam SourceOp The source class of the source dialect to convert from
     * @tparam DestOp The destination class of the target dialect to covert to
     */
    template <typename SourceOp, typename DestOp>
    class SimpleOneToToOneLoweringTemplate : public OpConversionPattern<SourceOp> {
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
            NewOpAndValues newOp = createNewOpAndValues(op, adaptor, rewriter);
            // We use the "replace op for values" method; This method check that the old op
            // yield the same number of SSA results as the given values
            rewriter.replaceOp(op, newOp.values);
            return success();
        }
    };

    namespace angle {
        extern std::string angleConversionFunctionName;

        bool moduleContainsAngleConversionDeclaration(ModuleOp &module);
        Operation *insertAngleConversionFunctionDeclaration(ModuleOp &module);
    } // namespace qoala::helpers::angle

    namespace print {
        /* Helper functions to print an operation recursively (i.e. including nested regions and ops) */
        struct IdentRAII {
            int &indent;
            IdentRAII(int &indent) : indent(indent) {}
            ~IdentRAII() { --indent; }
        };
        void printOperation(Operation *op);
        void printRegion(Region &region);
        void printBlock(Block &block);
        void resetIndent();
        IdentRAII pushIndent();

        llvm::raw_ostream &printIndent();
    } // namespace qoala::helpers::print
} // namespace qoala::helpers

#endif // QOALA_MLIR_HELPERS_H
