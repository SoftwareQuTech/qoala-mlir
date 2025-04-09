#include <Target/iQoala/iQoala.h>

#include "Analysis/NetQASM/Helpers.h"
#include "Dialect/NetQASM/NetQASM.h"

using namespace mlir;
using namespace qoala::dialects;
using namespace qoala::dialects::netqasm;

namespace qoala::analysis::netqasm {
    template <typename RoutineOpType>
    static Operation *getRoutineWithName(ModuleOp *mlirModule, const StringRef &functionName) {
        Operation *routineOp = nullptr;
        mlirModule->walk([&](RoutineOpType routine) -> WalkResult {
            if (routine.getSymNameAttr() == functionName) {
                routineOp = routine.getOperation();
                return WalkResult::interrupt();
            }
            return WalkResult::advance();
        });
        return routineOp;
    }

    bool hasLocalRoutineWithName(ModuleOp *mlirModule, const StringRef &functionName) {
        return getRoutineWithName<LocalRoutineOp>(mlirModule, functionName) != nullptr;
    }

    bool hasRequestRoutineWithName(ModuleOp *mlirModule, const StringRef &functionName) {
        return getRoutineWithName<RequestRoutineOp>(mlirModule, functionName) != nullptr;
    }

    Operation *getRoutineWithName(ModuleOp *mlirModule, const StringRef &functionName) {
        if (const auto localRoutine = getRoutineWithName<LocalRoutineOp>(mlirModule, functionName)) {
            return localRoutine;
        }
        return getRoutineWithName<RequestRoutineOp>(mlirModule, functionName);
    }

    std::map<uint32_t, uint8_t> getReturnedQubitsMap(ModuleOp *mlirModule, const StringRef &functionName,
        const iqoala::QuantumRoutine *quantumRoutine) {
        std::map<uint32_t, uint8_t> result;
        if (auto routineOp = dyn_cast_if_present<helpers::NetQASMRoutineInterface>(getRoutineWithName(mlirModule, functionName))) {
            const auto returnOp = routineOp.getReturnOperation();

            for (auto returnVal : llvm::enumerate(returnOp->getOperands())) {
                // Assumption: In general, local routines return either qubit references
                // OR measurement values, but NOT both (mixed value types).
                // Being this said, we have two options to check if the local routine
                // returns a qubit reference:
                // * An operand of the return op was yielded *inside* the local routine
                //   by a QAlloc operation.
                // * An operand is a function argument (returnVal.getDefiningOp() == nullptr)
                //   so we need to check in the QoalaHost section if the value is labeled as
                //   a qubit.
                if (auto definingOp = returnVal.value().getDefiningOp()) {
                    if (isa<QAllocOp>(definingOp)) {
                        result.emplace(returnVal.index(), quantumRoutine->getQubitNum(returnVal.value()));
                    }
                } else {
                    // TODO - The returned value is an argument. Trace it back to the
                    //  qoalahost section and check if it is marked as qubit there
                    assert(false && "Function returns an argument.");
                }
            }
        }
        return result;
    }

    bool blockArgIsQubit(const BlockArgument &blockArg) {
        // A naïve way to figure out if the function (block) argument is a qubit:
        // Check if they are used as operands on operations that accept qubits:
        for (auto &use : blockArg.getUses()) {
            Operation *user = use.getOwner();
            // These operations only use qubits as operands
            if (isa<QInitOp, MeasureOp, QFreeOp, HadamardOp, CnotOp, CzOp, EprsOp, EprsMeasureOp>(user)) {
                return true;
            }
            // There operations use qubits on the first operand
            if (isa<RotateXOp, RotateYOp, RotateZOp>(user) &&
                use.getOperandNumber() == 0) {
                return true;
            }
            // These operations use qubits on the first or second operand
            if (isa<CrotXOp>(user) && use.getOperandNumber() <= 1) {
                return true;
            }
        }
        return false;
    }

    template<typename RoutineType>
    static std::map<uint32_t, Value> getRoutineArgVals(RoutineType *routine) {
        std::map<uint32_t, Value> result;
        for (auto blockArg : routine->front().getArguments()) {
            result.emplace(blockArg.getArgNumber(), blockArg);
        }
        return result;
    }

    std::map<uint32_t, Value> getRoutineArgValues(Operation *routine) {
        if (auto localRoutine = dyn_cast<LocalRoutineOp>(routine)) {
            return getRoutineArgVals(&localRoutine);
        }
        if (auto requestRoutine = dyn_cast<RequestRoutineOp>(routine)) {
            return getRoutineArgVals(&requestRoutine);
        }
        return {};
    }
}