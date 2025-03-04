#ifndef QOALA_ANALYSIS_MLIR_HELPERS_H
#define QOALA_ANALYSIS_MLIR_HELPERS_H

#include "mlir/IR/Operation.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/Dialect/Tensor/IR/Tensor.h"
#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Transforms/DialectConversion.h"

namespace qoala::helpers {
    // This templated function is inspired by the implementation of llvm::isa<>()
    /**
     * Determines if the given operations belongs to the template types dialect.
     * @tparam Dialect The dialect class
     * @param operation The operation to test.
     * @return `true` if the operation belongs to the templated dialect type. `false` otherwise.
     */
    template<typename Dialect>
    bool belongsToDialect(mlir::Operation &operation) {
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
    bool belongsToDialect(mlir::Operation &operation) {
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
     * Configures the given ConversionTarget object to specify the valid state of the IR after
     * applying the QMem to QoalaHost dialect conversion.
     * @param target The ConversionTarget object to configure
     */
    void configureQMemToQoalaHostTarget(mlir::ConversionTarget &target,
                                        bool intRotsAreLegal,
                                        bool floatRotsAreLegal);

    /**
     * Adds the QMem to QoalaHost conversions patterns to the given rewrite pattern set.
     * It also uses the given type converter.
     * @param context The MLIRContext object.
     * @param patterns The pattern set object to populate.
     * @param typeConverter The type converter object used by the rewriter methods.
     */
    void populateQMemToQoalaHostPatterns(mlir::MLIRContext &context,
                                         mlir::RewritePatternSet &patterns,
                                         mlir::TypeConverter &typeConverter);

    /**
     * Configures the given ConversionTarget object to specify the valid state of the IR after
     * applying the QMem to NetQASM dialect conversion.
     * @param target The ConversionTarget object to configure
     */
    void configureQMemToNetQASMTarget(mlir::ConversionTarget &target);

    /**
     * Adds the QMem to NetQASM conversions patterns to the given rewrite pattern set.
     * It also uses the given type converter.
     * @param context The MLIRContext object.
     * @param patterns The pattern set object to populate.
     * @param typeConverter The type converter object used by the rewriter methods.
     */
    void populateQMemToNetQASMPatterns(mlir::MLIRContext &context,
                                       mlir::RewritePatternSet &patterns,
                                       mlir::TypeConverter &typeConverter);

    /**
     * Configures the given ConversionTarget object to specify the valid state of the IR after
     * applying the rotation operations conversion.
     * @param target The ConversionTarget object to configure
     */
    void configureF32LoweringTarget(mlir::ConversionTarget &target);

    /**
     * Adds the QMem to _intermediate_ QMem conversions patterns to the given rewrite pattern set.
     * It also uses the given type converter.
     * @param context The MLIRContext object.
     * @param patterns The pattern set object to populate.
     * @param typeConverter The type converter object used by the rewriter methods.
     */
    void populateQMemF32ToInt32RotPatterns(mlir::MLIRContext &context,
                                           mlir::RewritePatternSet &patterns,
                                           mlir::TypeConverter &typeConverter);

    /**
     * Configures the given ConversionTarget object to specify the valid state of the IR after
     * converting QMem (remote) to QRemote dialect.
     * @param target The ConversionTarget object to configure
     */
    void configureQMemToQRemoteTarget(mlir::ConversionTarget &target);

    /**
     * Adds the QMem to QRemote conversions patterns to the given rewrite pattern set.
     * It also uses the given type converter.
     * @param context The MLIRContext object.
     * @param patterns The pattern set object to populate.
     * @param typeConverter The type converter object used by the rewriter methods.
     */
    void populateQMemToQRemotePatterns(mlir::MLIRContext &context,
                                       mlir::RewritePatternSet &patterns,
                                       mlir::TypeConverter &typeConverter);

    /**
     * Simple "null" type converter for dialect conversion passes. This type
     * converter simply returns the same type for any given type.
     */
    class NullTypeConverter : public mlir::TypeConverter {
    public:
        explicit NullTypeConverter(mlir::MLIRContext *ctx);
    };

    template<typename OpTy>
    void moveOperationToTop(mlir::ModuleOp module, OpTy op) {
        if (op->getPrevNode() != nullptr) {
            // Simply remove the FuncOp, and insert it at the top of the module
            mlir::OpBuilder kk = mlir::OpBuilder::atBlockBegin(&module.getBodyRegion().front());
            op->remove();
            kk.insert(op);
        }
    }

    /**
     * Simple interface that defines a "print" method
     */
    struct PrintInterface {
    public:
        PrintInterface() = default;
        virtual ~PrintInterface() = default;
        virtual void print(mlir::raw_ostream &os) const = 0;
    };

    mlir::raw_ostream &operator<<(mlir::raw_ostream &os, const PrintInterface &printable);

    /**
     * Helper function that returns a string containing the "string" representation of each entry of
     * the given map, separated by a comma. Each entry is printed in the "{first} -> {second}" format.
     * To this end, the parametric types of the map members *must* implement the "<<" operator for the given type.
     * This must happen both for the Key and Value types.
     * @tparam KeyPrintableTy Type of the keys; this type needs to implement the PrintInterface interface
     * @tparam ValPrintableTy Type of the values; this type needs to implement the PrintInterface interface
     * @param socketsMap An std::map instance of `PrintableTy` type.
     * @return A string with all the members printed, separated with commas.
     */
    template<typename KeyPrintableTy, typename ValPrintableTy>
    std::string formatMap(const std::map<KeyPrintableTy, ValPrintableTy> &socketsMap) {
        std::stringstream result;
        for (auto &entry : socketsMap) {
            result << entry.second << " -> " << entry.first << ", ";
        }
        std::string partialResult = result.str();
        if (partialResult.empty()) {
            return partialResult;
        } else {
            return partialResult.substr(0, partialResult.length() - 2);
        }
    }

    /**
     * Helper function that returns a string containing the "string" representation of each member of
     * the given vector, separated by a comma. To this end, the parametric type of the vector members
     * *must* implement the "<<" operator for the given type.
     * @tparam PrintableTy
     * @param vector An std::vector instance of `PrintableTy` type.
     * @return A string with all the members printed, separated with commas.
     */
    template <typename PrintableTy>
    std::string formatVector(const std::vector<PrintableTy> &vector) {
        std::stringstream result;
        for (const PrintableTy &member : vector) {
            result << member << ", ";
        }
        std::string partialResult = result.str();
        if (partialResult.empty()) {
            return partialResult;
        } else {
            return partialResult.substr(0, partialResult.length() - 2);
        }
    }

    /**
     * Function that performs a data flow analysis on the `qoalahost.main_function` and
     * netqasm.local_routines, searching for "orphan" `arith.constant` operations (i.e.
     * operations that have no uses) and removes them.
     * @param module The module to analyze for orphan constant removal
     * @return Wether the analysis succeeded or failed
     */
    mlir::LogicalResult removeOrphanConstants(mlir::ModuleOp &module);
}

namespace qoala::translate {
    /**
     * Helper template used to create methods for registering dialect translations.
     * @tparam DialectTy Dialect class to translate
     * @tparam TranslationTy Class implementing the translation
     * @param registry The DialectRegistry instance to register the new translation class
     */
    template<typename DialectTy, typename TranslationTy>
    void registeriQoalaTranslation(mlir::DialectRegistry &registry) {
        registry.insert<DialectTy>();
        registry.addExtension(+[](mlir::MLIRContext *ctx, DialectTy *dialect) {
            dialect->template addInterfaces<TranslationTy>();
        });
    }
}
#endif //QOALA_ANALYSIS_MLIR_HELPERS_H
