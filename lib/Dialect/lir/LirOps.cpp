#include "mlir/IR/Attributes.h"
#include "mlir/IR/BlockAndValueMapping.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Support/LLVM.h"
#include "llvm/ADT/ArrayRef.h"

#include "Dialect/lir/LirOps.h"
#include "Dialect/lir/LirDialect.h"

using namespace mlir;
using namespace mlir::lir;

#define GET_OP_CLASSES
#include "Dialect/lir/LirOps.cpp.inc"

// void AddCValueOnQuanOp::build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::Value cin0, ::mlir::Value cin1)
// {
//     odsState.addOperands(cin0);
//     odsState.addOperands(cin1);
//     odsState.addTypes(lir::CValueOnQuanType());
// }