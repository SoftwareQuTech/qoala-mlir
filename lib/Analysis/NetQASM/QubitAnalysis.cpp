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

    template <typename RoutineOpType>
    static void eso(RoutineOpType routine) {
        LocalRoutineOp op;
        op.walk([&](QAllocOp qalloc) -> void {

        });
    }

    Operation *getLocalRoutineWithName(ModuleOp *mlirModule, const StringRef &functionName) {
        return getRoutineWithName<LocalRoutineOp>(mlirModule, functionName);
    }

    Operation *getRequestRoutineWithName(ModuleOp *mlirModule, const StringRef &functionName) {
        return getRoutineWithName<RequestRoutineOp>(mlirModule, functionName);
    }

    bool localRoutineReturnsQubit(ModuleOp *mlirModule, const StringRef &functionName) {
        return !getReturnedQubitIndexes(mlirModule, functionName).empty();
    }

    std::vector<uint32_t> getReturnedQubitIndexes(ModuleOp *mlirModule, const StringRef &functionName) {
        std::vector<uint32_t> result;
        if (auto localRoutineOp = dyn_cast_if_present<LocalRoutineOp>(getLocalRoutineWithName(mlirModule, functionName))) {
            const auto returnOps = localRoutineOp.getOps<ReturnOp>();
            assert(!returnOps.empty() && "LocalRoutineOp with no return statement.");

            auto returnOp = *returnOps.begin();
            for (auto returnVal : llvm::enumerate(returnOp.getOperands())) {
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
                        result.push_back(returnVal.index());
                    }
                } else {
                    // TODO - The returned value is an argument. Trace it back to the
                    //  qoalahost section and check if it is marked as qubit there
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
}