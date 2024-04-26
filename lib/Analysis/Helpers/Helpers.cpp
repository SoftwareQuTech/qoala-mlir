#include "Analysis/Helpers/Helpers.h"

using namespace mlir;
using namespace qoala::helpers;

/* Region verifiers for MainFuncOp */
bool qoala::helpers::operationIsNotFromCommonDialects(Operation &operation) {
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
// We also need to allow for "tensor" dialect. recv_ints/floats return tensors,
// and teh values NEED to be accessed with tensor.extract operation.
#define GET_OP_LIST
#include "mlir/Dialect/Tensor/IR/TensorOps.cpp.inc"
               >(operation));
}
