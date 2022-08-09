#include "mlir/IR/Attributes.h"
#include "mlir/IR/BlockAndValueMapping.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Support/LLVM.h"
#include "llvm/ADT/ArrayRef.h"

#include "Dialect/iqoala/iQoala.h"
#include "Dialect/iqoala/iQoalaDialect.h"

using namespace mlir;
using namespace mlir::iqoala;

#define GET_OP_CLASSES
#include "Dialect/iqoala/iQoala.cpp.inc"

LogicalResult ConstantOp::verify()
{
    return success();
}

void ConstantOp::build(OpBuilder &odsBuilder, OperationState &odsState, double value)
{
    build(odsBuilder, odsState, DenseElementsAttr());
}
