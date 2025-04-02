#ifndef HELPERS_H
#define HELPERS_H

#include "mlir/IR/BuiltinOps.h"

namespace qoala::analysis::netqasm {
    bool localRoutineReturnsQubit(mlir::ModuleOp *mlirModule, const mlir::StringRef &functionName);
}

#endif //HELPERS_H
