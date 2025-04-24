#ifndef HELPERS_H
#define HELPERS_H

#include "Target/iQoala/iQoala.h"
#include "mlir/IR/BuiltinOps.h"

namespace qoala::analysis::netqasm {
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

    class ArgValueMap {
    public:
        [[nodiscard]]
        mlir::Value getCallerValueForArg(const mlir::BlockArgument &blockArg) const;
        [[nodiscard]]
        mlir::BlockArgument getBlockArgForCallerValue(const mlir::Value &callerVal) const;
        void mapCallerArgToCalleeArgValue(const mlir::Value &callerVal, const mlir::BlockArgument &blockArg);
    private:
        mlir::DenseMap<mlir::Value, mlir::BlockArgument> callerArgsToCalleeArgMap;
        mlir::DenseMap<mlir::BlockArgument, mlir::Value> calleeArgsToCallerArgMap;
    };

    /**
     * Computes a map linking the argument values from a call operation with the BlockArgument values
     * that are used *inside* the called function.
     * @param routine The routine operation to analyze.
     * @param callOperands An OperandRange object that contains the argument values passed by the caller.
     * @return A map object that relates caller argument values and the BlockArgument values inside the called function.
     */
    ArgValueMap getRoutineArgValues(mlir::Operation *routine, const mlir::OperandRange &callOperands);

    /**
     * Returns a vector with all the instructions of the given opCode.
     * @param routine The Quantum routine to analyze
     * @param opCode The given OpCode to filter
     * @return A vector with all the MC instructions that load an argument
     */
    std::vector<assembly::iQoalaMCInstruction *> filterInstructionsFromRoutine(const iqoala::QuantumRoutine *routine,
        assembly::NetQASMMCInstr::OpCode opCode);
}

#endif //HELPERS_H
