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

    std::string getParentNetQASMRoutineName(mlir::Operation *op) {
        auto parent = op->getParentOfType<netqasm::LocalRoutineOp>();
        assert(parent != nullptr);
        return parent.getSymNameAttrName().str();
    }
}