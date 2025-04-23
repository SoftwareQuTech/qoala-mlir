#include "Dialect/Helpers/DialectHelpers.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Dialect/NetQASM/NetQASM.h"

using namespace mlir;

namespace qoala::dialects::helpers {
    bool operationIsInsideMainFunc(Operation *op) {
        const auto parent = op->getParentOfType<qoalahost::MainFuncOp>();
        return parent != nullptr;
    }

    bool operationIsInsideLocalRoutineFunc(Operation *op) {
        const auto parent = op->getParentOfType<netqasm::LocalRoutineOp>();
        return parent != nullptr;
    }

    bool operationIsInsideRequestRoutineFunc(Operation *op) {
        const auto parent = op->getParentOfType<netqasm::RequestRoutineOp>();
        return parent != nullptr;
    }

    std::string getParentLocalRoutineName(Operation *op) {
        // If the operation is a LocalRoutineOp, there is no need to look for a parent
        if (auto localRoutineOp = dyn_cast<netqasm::LocalRoutineOp>(op)) {
            return localRoutineOp.getSymName().str();
        }
        auto parent = op->getParentOfType<netqasm::LocalRoutineOp>();
        assert(parent != nullptr);
        return parent.getSymName().str();
    }

    std::string getParentRequestRoutineName(Operation *op) {
        // If the operation is a RequestRoutineOp, there is no need to look for a parent
        if (auto localRoutineOp = dyn_cast<netqasm::RequestRoutineOp>(op)) {
            return localRoutineOp.getSymName().str();
        }
        auto parent = op->getParentOfType<netqasm::RequestRoutineOp>();
        assert(parent != nullptr);
        return parent.getSymName().str();
    }


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
        return getRoutineWithName<netqasm::LocalRoutineOp>(mlirModule, functionName) != nullptr;
    }

    bool hasRequestRoutineWithName(ModuleOp *mlirModule, const StringRef &functionName) {
        return getRoutineWithName<netqasm::RequestRoutineOp>(mlirModule, functionName) != nullptr;
    }

    Operation *getRoutineWithName(ModuleOp *mlirModule, const StringRef &functionName) {
        if (const auto localRoutine = getRoutineWithName<netqasm::LocalRoutineOp>(mlirModule, functionName)) {
            return localRoutine;
        }
        return getRoutineWithName<netqasm::RequestRoutineOp>(mlirModule, functionName);
    }
}