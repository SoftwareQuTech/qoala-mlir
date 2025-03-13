#ifndef DIALECT_HELPERS_H
#define DIALECT_HELPERS_H

#include "mlir/IR/Operation.h"

namespace qoala::dialects::helpers {
    bool operationIsInsideMainFunc(mlir::Operation *op);
    bool operationIsInsideLocalRoutineFunc(mlir::Operation *op);
    std::string getParentNetQASMRoutineName(mlir::Operation *op);
}

#endif //DIALECT_HELPERS_H
