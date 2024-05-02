#ifndef QOALA_ANALYSIS_MLIR_HELPERS_H
#define QOALA_ANALYSIS_MLIR_HELPERS_H

#include "mlir/IR/Operation.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/Dialect/Tensor/IR/Tensor.h"
#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Transforms/DialectConversion.h"

using namespace mlir;

namespace qoala::helpers {

    // This templated function is inspired by the implementation of llvm::isa<>()
    /**
     * Determines if the given operations belongs to the template types dialect.
     * @tparam Dialect The dialect class
     * @param operation The operation to test.
     * @return `true` if the operation belongs to the templated dialect type. `false` otherwise.
     */
    template<typename Dialect>
    bool belongsToDialect(Operation &operation) {
        mlir::Dialect *operationDialect = operation.getDialect();
        return isa<Dialect>(operationDialect);
    }

    // This templated function is inspired by the implementation of llvm::isa<>()
    /**
     * Determines if the given operations belongs to one of the template types dialects.
     * @tparam DialectOne The first dialect class
     * @tparam DialectTwo A second dialect class
     * @tparam RestDialects The rest of the dialect classes
     * @param operation The operation to test.
     * @return `true` if the operation belongs to one of the templated dialect types. `false` otherwise.
     */
    template<typename DialectOne, typename DialectTwo, typename... RestDialects>
    bool belongsToDialect(Operation &operation) {
        mlir::Dialect *operationDialect = operation.getDialect();
        return isa<DialectOne>(operationDialect) || belongsToDialect<DialectTwo, RestDialects...>(operation);
    }

    // This templated function is inspired by the implementation of llvm::isa<>()
    /**
     * Returns a string that represents the C++ namespace of the parameter dialect.
     * @tparam Dialect The dialect class
     * @return A string with the C++ namespace of the given Dialect parameter class
     */
    template<typename Dialect>
    std::string getDialectNamesList() {
        std::string result;
        result += "'" + Dialect::getDialectNamespace().str() + "'";
        return result;
    }

    // This templated function is inspired by the implementation of llvm::isa<>()
    /**
     * Gets a string representing a list of the C++ namespaces of the given list of dialects.
     * @tparam DialectOne The first dialect class
     * @tparam DialectTwo A second dialect class
     * @tparam RestDialects The rest of the dialect classes
     * @return A comma-separated string with the C++ namespaces of the given dialect classes
     */
    template<typename DialectOne, typename DialectTwo, typename... RestDialects>
    std::string getDialectNamesList() {
        std::string result;
        result += "'" + DialectOne::getDialectNamespace().str() + "', "
                  + getDialectNamesList<DialectTwo, RestDialects...>();
        return result;
    }

    /* Helper functions to expose the conversion patterns from QMemToQoalaHost
     * and QMemtoNetQASM, to use them in the QoalaMIRToQoalaLIR general wrapper pass.
     * WARNING: The definitions of these functions are in the respective CPP files,
     * so they can be used both in the general MIR to LIR wrapper but also in the
     * passes they belong to.
     */

    /**
     * Adds the QMem to QoalaHost conversions patterns to the given rewrite pattern set.
     * It also uses the given type converter.
     * @param context The MLIRContext object.
     * @param patterns The pattern set object to populate.
     * @param typeConverter The type converter object used by the rewriter methods.
     */
    void populateQMemToQoalaHostPatterns(MLIRContext &context,
                                         RewritePatternSet &patterns,
                                         TypeConverter &typeConverter);

    /**
     * Adds the QMem to NetQASM conversions patterns to the given rewrite pattern set.
     * It also uses the given type converter.
     * @param context The MLIRContext object.
     * @param patterns The pattern set object to populate.
     * @param typeConverter The type converter object used by the rewriter methods.
     */
    void populateQMemToNetQASMPatterns(MLIRContext &context,
                                       RewritePatternSet &patterns,
                                       TypeConverter &typeConverter);

    /**
     * Simple "null" type converter for dialect conversion passes. This type
     * converter simply returns the same type for any given type.
     */
    class NullTypeConverter : public TypeConverter {
    public:
        explicit NullTypeConverter(MLIRContext *ctx);
    };
}


#endif //QOALA_ANALYSIS_MLIR_HELPERS_H
