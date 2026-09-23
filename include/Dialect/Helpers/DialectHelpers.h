#ifndef DIALECT_HELPERS_H
#define DIALECT_HELPERS_H

#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Operation.h"

namespace qoala::dialects::helpers {
    /**
     * Structure containing information about symbol (routine) names, and their
     * corresponding operations.
     */
    class RoutineMap {
    public:
        explicit RoutineMap(mlir::ModuleOp *module);

        [[nodiscard]]
        bool containsSymbolWithName(const mlir::StringRef &functionName) const;

        /**
         * Determines if there is a <b>local routine</b> with the given name in the analyzed MLIR module.
         * @param functionName The function name to search for.
         * @return `true` if there is a `LocalRoutineOp` with the given name. `false` otherwise.
         */
        [[nodiscard]]
        bool hasLocalRoutineWithName(const mlir::StringRef &functionName) const;

        /**
         * Gets a <b>local routine</b> with the given name in the analyzed MLIR module.
         * @param functionName The function name to search for.
         * @return A std::optional containing a `LocalRoutineOp` with the given name. The contained
         *         value can be safely cast to `LocalRoutineOp`.`std::nullopt` otherwise.
         */
        [[nodiscard]]
        std::optional<mlir::Operation *> getLocalRoutineWithName(const mlir::StringRef &functionName) const;

        /**
         * Determines if there is a <b>request routine</b> with the given name in the analyzed MLIR module.
         * @param functionName The function name to search for.
         * @return `true` if there is a `RequestRoutineOp` with the given name. `false` otherwise.
         */
        [[nodiscard]]
        bool hasRequestRoutineWithName(const mlir::StringRef &functionName) const;


        /**
         * Gets a <b>request routine</b> with the given name in the analyzed MLIR module.
         * @param functionName The function name to search for.
         * @return A std::optional containing a `RequestRoutineOp` with the given name. The contained
         *         value can be safely cast to `RequestRoutineOp`.`std::nullopt` otherwise.
         */
        [[nodiscard]]
        std::optional<mlir::Operation *> getRequestRoutineWithName(const mlir::StringRef &functionName) const;

        /**
         * Searches for a given <b>request or local routine</b> operation in the analyzed MLIR module. If not found, this
         * function returns `std::nullopt`.
         * @param functionName The function name to search for.
         * @return An `mlir::Operation` pointer to the found function, `std::nullopt` if a function with the given name was
         *         not found.
         */
        [[nodiscard]]
        std::optional<mlir::Operation *>getRoutineWithName(const mlir::StringRef &functionName) const;

    private:
        llvm::StringMap<mlir::Operation *> localRoutinesMap;
        llvm::StringMap<mlir::Operation *> requestRoutinesMap;
    };

    bool operationIsInsideMainFunc(mlir::Operation *op);
    bool operationIsInsideLocalRoutineFunc(mlir::Operation *op);
    bool operationIsInsideRequestRoutineFunc(mlir::Operation *op);
    std::string getParentLocalRoutineName(mlir::Operation *op);
    std::string getParentRequestRoutineName(mlir::Operation *op);

    /**
     * Determines if there is a <b>local routine</b> operation in the given MLIR module.
     * @param mlirModule The MLIR module to search for the function.
     * @param functionName The function name to search for.
     * @return `true` if there is a `LocalRoutineOp` with the given name. `false` otherwise.
     */
    bool hasLocalRoutineWithName(mlir::ModuleOp *mlirModule, const mlir::StringRef &functionName);

    /**
     * Determines if there is a <b>local routine</b> operation in the given MLIR module.
     * @param mlirModule The MLIR module to search for the function.
     * @param functionName The function name to search for.
     * @return `true` if there is a `LocalRoutineOp` with the given name. `false` otherwise.
     */
    bool hasRequestRoutineWithName(mlir::ModuleOp *mlirModule, const mlir::StringRef &functionName);

    /**
     * Searches for a given <b>request or local routine</b> operation in the given MLIR module. If not found, this
     * function returns `nullptr`.
     * @param mlirModule The MLIR module to search for the function.
     * @param functionName The function name to search for.
     * @return An `mlir::Operation` pointer to the found function, `nullptr` if a function with the given name was
     *         not found.
     */
    mlir::Operation *getRoutineWithName(mlir::ModuleOp *mlirModule, const mlir::StringRef &functionName);
} // namespace qoala::dialects::helpers

#endif // DIALECT_HELPERS_H
