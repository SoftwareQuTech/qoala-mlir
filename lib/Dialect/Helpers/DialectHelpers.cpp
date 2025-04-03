#include "Dialect/Helpers/DialectHelpers.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Dialect/NetQASM/NetQASM.h"

namespace qoala::dialects::helpers {
    bool operationIsInsideMainFunc(mlir::Operation *op) {
        const auto parent = op->getParentOfType<qoalahost::MainFuncOp>();
        return parent != nullptr;
    }

    bool operationIsInsideLocalRoutineFunc(mlir::Operation *op) {
        const auto parent = op->getParentOfType<netqasm::LocalRoutineOp>();
        return parent != nullptr;
    }

    bool operationIsInsideRequestRoutineFunc(mlir::Operation *op) {
        const auto parent = op->getParentOfType<netqasm::RequestRoutineOp>();
        return parent != nullptr;
    }

    std::string getParentLocalRoutineName(mlir::Operation *op) {
        auto parent = op->getParentOfType<netqasm::LocalRoutineOp>();
        assert(parent != nullptr);
        return parent.getSymName().str();
    }

    std::string getParentRequestRoutineName(mlir::Operation *op) {
        auto parent = op->getParentOfType<netqasm::RequestRoutineOp>();
        assert(parent != nullptr);
        return parent.getSymName().str();
    }
}