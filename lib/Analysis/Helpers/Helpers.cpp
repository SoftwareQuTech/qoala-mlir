#include "Analysis/Helpers/Helpers.h"

using namespace mlir;
using namespace qoala::helpers;

/* Region verifiers for MainFuncOp */
bool qoala::helpers::operationIsNotFromAllowedDialects(Operation &operation) {
    return ! (isa<
#define GET_OP_LIST
#include "mlir/Dialect/Arith/IR/ArithOps.cpp.inc"
              >(operation) ||
              isa<
#define GET_OP_LIST
#include "mlir/Dialect/MemRef/IR/MemRefOps.cpp.inc"
              >(operation) ||
              isa<
#define GET_OP_LIST
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.cpp.inc"
              >(operation) ||
              isa<
#define GET_OP_LIST
#include "mlir/Dialect/Tensor/IR/TensorOps.cpp.inc"
              >(operation));
}
