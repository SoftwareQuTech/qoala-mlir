#ifndef HELPERS_H
#define HELPERS_H

#include "mlir/IR/BuiltinOps.h"

namespace qoala::analysis::netqasm {
    /**
     * Searches for a given <b>local routine</b> operation in the given MLIR module. If not found, this
     * function returns `nullptr`.
     * @param mlirModule The MLIR module to search for the function.
     * @param functionName The function name to search for.
     * @return An `mlir::Operation` pointer to the found function, `nullptr` if a function with the name was
     *         not found. The returned pointer can safely be casted to `qoala::dialects::netqasm::LocalRoutineOp`.
     */
    mlir::Operation *getLocalRoutineWithName(mlir::ModuleOp *mlirModule, const mlir::StringRef &functionName);

    /**
     * Searches for a given <b>request routine</b> operation in the given MLIR module. If not found, this
     * function returns `nullptr`.
     * @param mlirModule The MLIR module to search for the function.
     * @param functionName The function name to search for.
     * @return An `mlir::Operation` pointer to the found function, `nullptr` if a function with the name was
     *         not found. The returned pointer can safely be casted to `qoala::dialects::netqasm::RequestRoutineOp`.
     */
    mlir::Operation *getRequestRoutineWithName(mlir::ModuleOp *mlirModule, const mlir::StringRef &functionName);

    /**
     * Looks for a function with the given name inside the given MLIR module, and checks whether
     * that function returns one or more qubits or not. If a function with the given name could
     * not be found in the module, this function returns `false`.
     * @param mlirModule The MLIR module to search for the function.
     * @param functionName The function name to analyze.
     * @return Whether the given function returns a pointer or not. If a function with the given
     *         name is not found in the module, this function returns `false`.
     */
    bool localRoutineReturnsQubit(mlir::ModuleOp *mlirModule, const mlir::StringRef &functionName);

    /**
     * Computes the indexes of the return values that represent a qubit value.
     * @param mlirModule The MLIR module to search for the function.
     * @param functionName The function name to analyze.
     * @return A vector with indexes of the return values that represent a qubit
     */
    std::vector<uint32_t> getReturnedQubitIndexes(mlir::ModuleOp *mlirModule, const mlir::StringRef &functionName);

    /**
     * Determines if the given block argument (value) <b>is used</b> as a qubit or not.
     * This is a strong indicator that the value is a qubit reference.
     * @param blockArg The block argument to analyze.
     * @return Whether the argument is a qubit or not.
     */
    bool blockArgIsQubit(const mlir::BlockArgument &blockArg);
}

#endif //HELPERS_H
