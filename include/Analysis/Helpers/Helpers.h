#ifndef QOALA_ANALYSIS_MLIR_HELPERS_H
#define QOALA_ANALYSIS_MLIR_HELPERS_H

#include "llvm/Support/Casting.h"

#if __cplusplus >= 202002L
#include <format>
#endif

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/IR/Operation.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Transforms/FoldUtils.h"

#include <set>
#include <vector>

namespace qoala::dialects::qmem {
    // Forward declaration
    class FuncOp;
} // namespace qoala::dialects::qmem

namespace qoala::helpers {
    // This templated function is inspired by the implementation of llvm::isa<>()
    /**
     * Determines if the given operation belongs to the template types dialect.
     * @tparam Dialect The dialect class
     * @param operation The operation to test.
     * @return `true` if the operation belongs to the templated dialect type. `false` otherwise.
     */
    template<class Dialect>
    bool belongsToDialect(mlir::Operation &operation) {
        mlir::Dialect *operationDialect = operation.getDialect();
        return llvm::isa<Dialect>(operationDialect);
    }

    // This templated function is inspired by the implementation of llvm::isa<>()
    /**
     * Determines if the given operation belongs to one of the template types dialects.
     * @tparam DialectOne The first dialect class
     * @tparam DialectTwo A second dialect class
     * @tparam RestDialects The rest of the dialect classes
     * @param operation The operation to test.
     * @return `true` if the operation belongs to one of the templated dialect types. `false` otherwise.
     */
    template<class DialectOne, class DialectTwo, class... RestDialects>
    bool belongsToDialect(mlir::Operation &operation) {
        mlir::Dialect *operationDialect = operation.getDialect();
        return llvm::isa<DialectOne>(operationDialect) || belongsToDialect<DialectTwo, RestDialects...>(operation);
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
        result += "'" + DialectOne::getDialectNamespace().str() + "', " +
                  getDialectNamesList<DialectTwo, RestDialects...>();
        return result;
    }

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
     * Formats a given format string using the given values. IMPORTANT: The format string style
     * depends on the C++ standard used: For C++17 or older, use the C-style format string (e.g.
     * "my_format_%d". For C++20 or newer, use "python"-style format string (e.g. "my_format_{}").
     * @tparam Args The argument types to fill in the format string. These should be automagically
     *              deducted when using this function.
     * @param fmt The format string. If using C++17 or older standard, use a C-style format string.
     *            If using C++20 or later, use a python-style format string.
     * @param args The values to fill in the format string.
     * @return A string with the tokens replaced with the given values
     */
    template<typename... Args>
    std::string formatString(const std::string &fmt, Args &&...args) {
        /* When using C++20 or later standard, we can make use of the "format" header,
         * and easily format the string */
#if __cplusplus >= 202002L
        return std::vformat(std::string_view(fmt), std::make_format_args(args...));
#else
        /* In older versions of the standard, we need to default to the good'ol C-way
         * of formatting a string */
        const int length = std::snprintf(nullptr, 0, fmt.c_str(), args...);
        std::vector<char> formattedString(length + 1);
        std::sprintf(formattedString.data(), fmt.c_str(), args...);
        return {formattedString.data()};
#endif
    }

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
    template<typename PrintableTy>
    std::string formatVector(const std::vector<PrintableTy> &vector) {
        std::stringstream result;
        for (const PrintableTy &member : vector) {
            result << member << ", ";
        }
        if (std::string partialResult = result.str(); partialResult.empty()) {
            return partialResult;
        } else {
            return partialResult.substr(0, partialResult.length() - 2);
        }
    }

    /**
     * Helper function that returns a string containing the "string" representation of each key-value pair in
     * the given unordered_map, formatted as "key: value", and separated by commas. Both the key and the value types
     * must support the "<<" operator.
     *
     * @tparam KeyTy Type of the keys in the map.
     * @tparam ValueTy Type of the values in the map.
     * @param map An std::unordered_map instance of <KeyTy, ValueTy>.
     * @return A string with all key-value pairs printed as "key: value", separated with commas.
     */
    template<typename KeyTy, typename ValueTy>
    std::string formatUnorderedMap(const std::unordered_map<KeyTy, ValueTy> &map) {
        std::stringstream result;
        for (const auto &pair : map) {
            result << pair.first << ": " << pair.second << ", ";
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
     * the given set, separated by a comma. To this end, the parametric type of the vector members
     * *must* implement the "<<" operator for the given type.
     * @tparam PrintableTy
     * @param vector An std::vector instance of `PrintableTy` type.
     * @return A string with all the members printed, separated with commas.
     */
    template<typename PrintableTy>
    std::string formatSet(const std::set<PrintableTy> &vector) {
        std::stringstream result;
        for (const PrintableTy &member : vector) {
            result << member << ", ";
        }
        if (std::string partialResult = result.str(); partialResult.empty()) {
            return partialResult;
        } else {
            return partialResult.substr(0, partialResult.length() - 2);
        }
    }

    /**
     * Small class that keeps that of all the constants that are marked for removal
     * It implements methods from RewriterBase::Listener to keep track of the folder instructions
     * and instructions that can be safely removed after folding.
     */
    class FolderTracker : public mlir::RewriterBase::Listener {
        // All constants in the operation post folding.
        std::vector<mlir::Operation *> existingConstants;

    public:
        std::vector<mlir::Operation *> getExistingConstants() { return existingConstants; }

        void notifyOperationInserted(mlir::Operation *op) override { existingConstants.push_back(op); }

        void notifyOperationRemoved(mlir::Operation *op) override {
            if (const auto it = llvm::find(existingConstants, op); it != existingConstants.end()) {
                existingConstants.erase(it);
            }
        }
    };

    /**
     * Simple analysis method that folds instructions statically if it is possible.
     * This is used for folding constants (arith.constants declared statically, operated
     * and then used within the same scope) and remove them to simplify any further analysis.
     * @param op The operation to analyze for constant folding
     * @return Weather the analysis succeeded or failed
     */
    template<typename OpTy>
    mlir::LogicalResult foldConstants(OpTy &op) {
        std::vector<mlir::Operation *> ops;
        FolderTracker folderTracker;
        mlir::OperationFolder folderHelper(op.getContext(), /*listener=*/&folderTracker);

        // We just walk over all the instructions, discovering them for potential folding
        op.template walk<mlir::WalkOrder::PreOrder>([&](mlir::Operation *operation) { ops.push_back(operation); });

        // Visit the discovered ops in reverse order, so we don't break data dependencies
        for (mlir::Operation *operation : llvm::reverse(ops)) {
            (void) folderHelper.tryToFold(operation);
        }

        // Finally, remove all orphaned constants after folding them
        for (const auto cst : folderTracker.getExistingConstants()) {
            if (cst->use_empty()) {
                cst->erase();
            }
        }
        return mlir::success();
    }

    enum class RotationAxis { X, Y, Z };
} // namespace qoala::helpers
#endif // QOALA_ANALYSIS_MLIR_HELPERS_H
