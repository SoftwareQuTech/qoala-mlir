#ifndef QOALA_MLIR_HELPERS_H
#define QOALA_MLIR_HELPERS_H

#include "llvm/Support/Debug.h"
#include "mlir/Transforms/DialectConversion.h"

namespace qoala::helpers {
    struct OpAndValues {
        // OpAndValues MUST own the returned mlir::Value objects.
        //
        // mlir::ValueRange / mlir::OperandRange are non-owning views. Some lowerings
        // were returning ranges backed by temporaries (e.g. OperandRange slices or
        // other short-lived range objects). During dialect conversion this can create
        // dangling references and corrupt replacement values, leading to invalid
        // defining-op pointers, bogus operand/result counts, verifier failures, and
        // crashes during isa<>, printing, or type handling.
        //
        // To avoid these lifetime issues, OpAndValues eagerly copies all replacement
        // values into an owning container.
        OpAndValues(mlir::Operation *op, mlir::ValueRange vr): operation(op), values(vr.begin(), vr.end()) { }

        OpAndValues(mlir::Operation *op, mlir::Value v): operation(op), values{v} { }

        // Convenience overload: accept operand ranges too.
        OpAndValues(mlir::Operation *op, mlir::OperandRange orng): operation(op) {
            values.reserve(orng.size());
            for (mlir::Value v : orng) {
                values.push_back(v);
            }
        }

        mlir::Operation *operation{};
        llvm::SmallVector<mlir::Value, 4> values;
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
    template<typename SourceOp, typename... DestOps>
    class OpLoweringTemplate : public mlir::OpConversionPattern<SourceOp> {
    public:
        // Constructor that call the super class and sets the flag to allow
        // applying this rewrite pattern multiple times in a single recursion stack
        OpLoweringTemplate(const mlir::TypeConverter &typeConverter, mlir::MLIRContext *context):
            mlir::OpConversionPattern<SourceOp>(typeConverter, context) {
            this->setHasBoundedRewriteRecursion();
        }

        /**
         * Defines how to create an Operation on the destination dialect, using
         * the data offered by the adaptor class of the source operation.
         * Implementors of this signature are *highly* encouraged to use the
         * `create` methods of the rewriter object to create instances of the
         * operation on the target dialect. When using these methods, please be
         * mindful that you need to "match" one of the target operations' class
         * `build` methods with the arguments passed to the create method:
         * rewriter.create<DestOp>(op.getLoc(), <args_to_match_a_build_signature>...)
         * In this sense, you might want to use the getters of the adaptor class
         * to get the data (and types) to match any of the generated builders of the
         * target operand class.
         * IMPORTANT: The returned operation *MUST* yield the same number of values
         * than the operation to be replaced. If this is not the case, this class is not
         * suitable for the purpose.
         * IMPORTANT: When using the adaptor, save the returned values in a local variable,
         * and use those instead of constantly asking for the same to the adaptor. This is
         * due to the fact that MLIR can decide to run conversion patterns in parallel.
         * This will lead to data race, since the adaptor always has "the most updated
         * version of the adapted value". IF you ask for a value only once, it should not
         * be a problem.
         * @param op The operation on the source dialect to be replaced
         * @param adaptor The adaptor of th source operation to get data from
         * @param rewriter The rewriter object to easily create new operations on the
         *                 target dialect
         * @return A smart pointer object of type OpAndValues, which contains a pointer
         *         to the created operation (as returned by getOperation()) and the new
         *         values to replace in the users of the old operation.
         */
        virtual std::unique_ptr<OpAndValues> createNewOpAndValues(SourceOp op, typename SourceOp::Adaptor adaptor,
                                                                  mlir::ConversionPatternRewriter &rewriter) const = 0;

        mlir::LogicalResult matchAndRewrite(SourceOp op, typename SourceOp::Adaptor adaptor,
                                            mlir::ConversionPatternRewriter &rewriter) const override {
            const std::unique_ptr<OpAndValues> newOpAndVals = createNewOpAndValues(op, adaptor, rewriter);
            // Expect an operation of a type of the declared destination types
            assert(llvm::isa<DestOps...>(newOpAndVals->operation));
            // Replace the old op with the values produced by the lowering.
            // Note: MLIR APIs take ValueRange (a non-owning view). OpAndValues now owns the
            // underlying Values, so it is safe to wrap them here.
            rewriter.replaceOp(op, mlir::ValueRange(newOpAndVals->values));
            return mlir::success();
        }
    };

    namespace angle {
        extern std::string angleConversionFunctionName;

        bool moduleContainsAngleConversionDeclaration(mlir::ModuleOp &module);
        mlir::Operation *insertAngleConversionFunctionDeclaration(mlir::ModuleOp &module);
        std::vector<uint32_t> transformDouble(double angleRads);
    } // namespace angle

    namespace print {
        /* Helper functions to print an operation recursively (i.e. including nested regions and ops) */
        struct IdentRAII {
            uint32_t &indent;

            explicit IdentRAII(uint32_t &indent): indent(indent) { }

            ~IdentRAII() { --indent; }
        };

        void printOperation(mlir::Operation *op);
        void printRegion(mlir::Region &region);
        void printBlock(mlir::Block &block);
        void resetIndent();
        IdentRAII pushIndent();

        llvm::raw_ostream &printIndent();
    } // namespace print
} // namespace qoala::helpers

#endif // QOALA_MLIR_HELPERS_H
