#ifndef QOALA_MLIR_HELPERS_H
#define QOALA_MLIR_HELPERS_H

#include "mlir/Transforms/DialectConversion.h"
#include "llvm/Support/Debug.h"

namespace qoala::helpers {
    struct OpAndValues {
        mlir::Operation *operation{};
        mlir::ValueRange values;
    };

    /* Helper template class used to create simple operation rewriter classes */
    /**
     * Class template used to convert an operation of class SourceOp to
     * another one of class DestOp, *where `DestOp` produces a different number
     * of values than `SourceOp`*.
     * @tparam SourceOp The source class of the source dialect to convert from
     * @tparam DestOps The destination class(es) of the target dialect to covert to.
     *                 If multiple classes are passed then the operation must have
     *                 _one_ of the given destination types
     */
    template <typename SourceOp, typename... DestOps>
    class OpLoweringTemplate : public mlir::OpConversionPattern<SourceOp> {
    public:
        // Constructor simply matches the super class
        using mlir::OpConversionPattern<SourceOp>::OpConversionPattern;

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
         * @return A smart pointer object of type OpAndValues, which contains a pointer
         *         to the created operation (as returned by getOperation()) and the new
         *         values to replace in the users of the old operation.
         */
        virtual std::unique_ptr<OpAndValues>
                createNewOpAndValues(SourceOp op, typename SourceOp::Adaptor adaptor,
                                     mlir::ConversionPatternRewriter &rewriter) const = 0;

        mlir::LogicalResult
        matchAndRewrite(SourceOp op, typename SourceOp::Adaptor adaptor,
                        mlir::ConversionPatternRewriter &rewriter) const override {
            std::unique_ptr<OpAndValues> newOpAndVals = createNewOpAndValues(op, adaptor, rewriter);
            // Expect an operation of a type of the declared destination types
            assert(llvm::isa<DestOps...>(newOpAndVals->operation));
            // We use the "replace op for values" method; This method check that the old op
            // yield the same number of SSA results as the given values
            rewriter.replaceOp(op, newOpAndVals->values);
            return mlir::success();
        }
    };

    namespace angle {
        extern std::string angleConversionFunctionName;

        bool moduleContainsAngleConversionDeclaration(mlir::ModuleOp &module);
        mlir::Operation *insertAngleConversionFunctionDeclaration(mlir::ModuleOp &module);
        std::vector<uint32_t> transformDouble(double angleRads);
    } // namespace qoala::helpers::angle

    namespace print {
        /* Helper functions to print an operation recursively (i.e. including nested regions and ops) */
        struct IdentRAII {
            int &indent;
            IdentRAII(int &indent) : indent(indent) {}
            ~IdentRAII() { --indent; }
        };
        void printOperation(mlir::Operation *op);
        void printRegion(mlir::Region &region);
        void printBlock(mlir::Block &block);
        void resetIndent();
        IdentRAII pushIndent();

        llvm::raw_ostream &printIndent();
    } // namespace qoala::helpers::print
} // namespace qoala::helpers

#endif // QOALA_MLIR_HELPERS_H
