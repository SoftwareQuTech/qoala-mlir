#ifndef HELPERS_H
#define HELPERS_H

#include "Target/iQoala/iQoala.h"
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
     * Looks for a function with the given name inside the given MLIR module, and creates a map between
     * the indexes of the returned MLIR values thar represent a qubit and the physical qubit IDs allocated
     * for those values.
     * @param mlirModule The MLIR module to search for the function.
     * @param functionName The function name to analyze.
     * @param quantumRoutine The iQoala object that represents the quantum routine to analyze.
     * @return A map that relates the returned indexes of qubits with the physical qubit IDs allocated
     *         within the body of the MLIR quantum routine.
     */
    std::map<uint32_t, uint8_t> getReturnedQubitsMap(mlir::ModuleOp *mlirModule, const mlir::StringRef &functionName,
        const iqoala::QuantumRoutine *quantumRoutine);

    /**
     * Determines if the given block argument (value) <b>is used</b> as a qubit or not.
     * This is a strong indicator that the value is a qubit reference.
     * @param blockArg The block argument to analyze.
     * @return Whether the argument is a qubit or not.
     */
    bool blockArgIsQubit(const mlir::BlockArgument &blockArg);

    /**
     * Computes a map linking the indexes of the arguments and the MLIR values *inside* the
     * routine body.
     * @param routine The routine operation to analyze
     * @return A map between the argument indexes and the respective MLIR values
     */
    std::map<uint32_t, mlir::Value> getRoutineArgValues(mlir::Operation *routine);
}

#endif //HELPERS_H
