#include "mlir/IR/Attributes.h"
#include "mlir/IR/BlockAndValueMapping.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Support/LLVM.h"
#include "llvm/ADT/ArrayRef.h"

#include "Dialect/hir/HirOps.h"

using namespace mlir;
using namespace mlir::hir;

#define GET_OP_CLASSES
#include "Dialect/hir/HirOps.cpp.inc"
