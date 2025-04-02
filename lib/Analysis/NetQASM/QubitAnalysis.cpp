#include "Analysis/NetQASM/Helpers.h"
#include "Dialect/NetQASM/NetQASM.h"

using namespace mlir;
using namespace qoala::dialects;

namespace qoala::analysis::netqasm {
    bool localRoutineReturnsQubit(ModuleOp *mlirModule, const StringRef &functionName) {
        // TODO
        dialects::netqasm::LocalRoutineOp *localRoutineOp = nullptr;

        mlirModule->walk([&](dialects::netqasm::LocalRoutineOp localRoutine) -> void {
            if (localRoutine.getSymNameAttr() == functionName) {
                localRoutineOp = &localRoutine;
            }
        });
        if (!localRoutineOp) {
            return false;
        }
        dialects::netqasm::ReturnOp *returnOp = nullptr;
        localRoutineOp->walk([&](dialects::netqasm::ReturnOp retOp) -> void {
            returnOp = &retOp;
        });
        assert(returnOp && "LocalRoutineOp with no return statement.");
        for (auto returnVal : returnOp->getOperands()) {
            // Assumption: In general, local routines return either qubit references
            // OR measurement values, but NOT both (mixed value types).
            // Being this said, we have two options to check if the local routine
            // returns a qubit reference:
            // * An operand of the return op was yielded *inside* the local routine
            //   by a QAlloc operation.
            // * An operas is a function argument (returnVal.getDefiningOp() == nullptr)
            //   so we need to check in the QoalaHost section if the value is labeled as
            //   a qubit.
            if (auto definingOp = returnVal.getDefiningOp()) {
                if (isa<dialects::netqasm::QAllocOp>(definingOp)) {
                    return true;
                }
            } else {
                // TODO - The returned value is an argument. Trace it back to the
                //  qoalahost section and check if it is marked as qubit there
            }
        }
        return false;
    }
}