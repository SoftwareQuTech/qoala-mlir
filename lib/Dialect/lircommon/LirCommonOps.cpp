#include "mlir/IR/Attributes.h"
#include "mlir/IR/BlockAndValueMapping.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Support/LLVM.h"
#include "llvm/ADT/ArrayRef.h"

#include "Dialect/lircommon/LirCommonOps.h"
#include "Dialect/lircommon/LirCommonDialect.h"

using namespace mlir;
using namespace mlir::lircommon;

#define GET_OP_CLASSES
#include "Dialect/lircommon/LirCommonOps.cpp.inc"
