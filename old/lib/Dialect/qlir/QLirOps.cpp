#include "mlir/IR/Attributes.h"
#include "mlir/IR/BlockAndValueMapping.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Support/LLVM.h"
#include "llvm/ADT/ArrayRef.h"

#include "Dialect/qlir/QLirOps.h"
#include "Dialect/qlir/QLirDialect.h"

#include "Dialect/lir/LirTypes.h"
#include "Dialect/lir/LirDialect.h"

using namespace mlir;
using namespace mlir::qlir;

#define GET_OP_CLASSES
#include "Dialect/qlir/QLirOps.cpp.inc"
