#include "Analysis/Helpers/Helpers.h"
#include <string>

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
// We also need to allow the "tensor" dialect. recv_ints/floats return tensors,
// and teh values NEED to be accessed with tensor.extract operation.
#define GET_OP_LIST
#include "mlir/Dialect/Tensor/IR/TensorOps.cpp.inc"
               >(operation) ||
               isa<
// We also need to allow the "affine" dialect. This is useful to support loops
#define GET_OP_LIST
#include "mlir/Dialect/Affine/IR/AffineOps.cpp.inc"
               >(operation));
}

std::string qoala::helpers::getAllowedDialectNames() {
    std::string result;
    result += "'" + arith::ArithDialect::getDialectNamespace().str() + "', '"
                  + memref::MemRefDialect::getDialectNamespace().str() + "', '"
                  + cf::ControlFlowDialect::getDialectNamespace().str() + "', '"
                  + tensor::TensorDialect::getDialectNamespace().str() + "', '"
                  + affine::AffineDialect::getDialectNamespace().str() + "' ";
    return result;
}


/* Implementation of the null types converter */
qoala::helpers::NullTypeConverter::NullTypeConverter(MLIRContext *ctx) {
    // For all types, we map it to the same instance
    this->addConversion([](Type type) { return type; });
}
