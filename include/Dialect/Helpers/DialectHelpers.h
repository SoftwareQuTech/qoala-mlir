#ifndef DIALECT_HELPERS_H
#define DIALECT_HELPERS_H

#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Operation.h"

namespace qoala::dialects::helpers {
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
