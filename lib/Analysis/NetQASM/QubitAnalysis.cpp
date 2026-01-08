#include "Analysis/NetQASM/Helpers.h"
#include "Dialect/Helpers/DialectHelpers.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "Target/iQoala/iQoala.h"

using namespace mlir;
using namespace qoala::dialects;
using namespace qoala::dialects::helpers;
using namespace qoala::dialects::netqasm;

namespace qoala::analysis::netqasm {
    std::map<uint32_t, uint8_t> getReturnedQubitsMap(ModuleOp *mlirModule, const StringRef &functionName,
                                                     const iqoala::QuantumRoutine *quantumRoutine) {
        std::map<uint32_t, uint8_t> result;
        if (auto routineOp = dyn_cast_if_present<helpers::NetQASMRoutineInterface>(
                    getRoutineWithName(mlirModule, functionName))) {
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
                        uint8_t qubitID = quantumRoutine->getQubitNum(returnVal.value());
                        result.emplace(returnVal.index(), qubitID);
                    }
                } else {
                    // TODO - The returned value is an argument. Trace it back to the
                    //  qoalahost section and check if it is marked as qubit there?
                    assert(false && "Function returns an argument.");
                }
            }
        }
        return result;
    }

    bool blockArgIsQubit(const BlockArgument &blockArg) {
        // A naïve way to figure out if the function (block) argument is a qubit:
        // Check if they are used as operands on operations that accept qubits:
        const auto uses = blockArg.getUses();
        return std::any_of(uses.begin(), uses.end(), [](OpOperand &use) -> bool {
            const Operation *user = use.getOwner();
            // These operations only use qubits as operands
            if (isa<QInitOp, QFreeOp, ifaces::SingleQubitOp>(user) && use.getOperandNumber() == 0) {
                return true;
            }
            // These operations use qubits on the first or second operand
            if (isa<ifaces::DualQubitOp>(user) && use.getOperandNumber() <= 1) {
                return true;
            }
            return false;
        });
    }

    Value ArgValueMap::getCallerValueForArg(const BlockArgument &blockArg) const {
        return this->calleeArgsToCallerArgMap.at(blockArg);
    }

    BlockArgument ArgValueMap::getBlockArgForCallerValue(const Value &callerVal) const {
        return this->callerArgsToCalleeArgMap.at(callerVal);
    }

    void ArgValueMap::mapCallerArgToCalleeArgValue(const Value &callerVal, const BlockArgument &blockArg) {
        const auto result = this->callerArgsToCalleeArgMap.try_emplace(callerVal, blockArg);
        (void) result;
        assert(result.second && "Attempting to map a caller value that is already mapped");
        const auto resultB = this->calleeArgsToCallerArgMap.try_emplace(blockArg, callerVal);
        (void) resultB;
        assert(resultB.second && "Attempting to map a block argument that is already mapped");
    }

    template<typename RoutineType>
    static ArgValueMap getRoutineArgVals(RoutineType routine, const OperandRange &callOperands) {
        ArgValueMap result;
        assert(routine.getNumArguments() == callOperands.size() &&
               "Caller operand count doesn't match the callee arguments count");
        for (auto blockArg : routine.front().getArguments()) {
            result.mapCallerArgToCalleeArgValue(callOperands[blockArg.getArgNumber()], blockArg);
        }
        return result;
    }

    ArgValueMap getRoutineArgValues(Operation *routine, const OperandRange &callOperands) {
        if (const auto localRoutine = dyn_cast<LocalRoutineOp>(routine)) {
            return getRoutineArgVals(localRoutine, callOperands);
        }
        if (const auto requestRoutine = dyn_cast<RequestRoutineOp>(routine)) {
            return getRoutineArgVals(requestRoutine, callOperands);
        }
        return {};
    }
} // namespace qoala::analysis::netqasm
