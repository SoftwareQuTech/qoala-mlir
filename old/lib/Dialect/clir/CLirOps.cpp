#include "mlir/IR/Attributes.h"
#include "mlir/IR/BlockAndValueMapping.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Support/LLVM.h"
#include "llvm/ADT/ArrayRef.h"

#include "Dialect/clir/CLirOps.h"
#include "Dialect/clir/CLirDialect.h"

using namespace mlir;
using namespace mlir::clir;

#define GET_OP_CLASSES
#include "Dialect/clir/CLirOps.cpp.inc"
