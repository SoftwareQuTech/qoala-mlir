#include "mlir/IR/Attributes.h"
#include "mlir/IR/BlockAndValueMapping.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Support/LLVM.h"
#include "llvm/ADT/ArrayRef.h"
#include "mlir/IR/FunctionImplementation.h"

#include "Dialect/qoala_lir/QoalaLir.h"
#include "Dialect/qoala_lir/QoalaLirDialect.h"

using namespace mlir;
using namespace mlir::qoala_lir;

#define GET_OP_CLASSES

#include "Dialect/qoala_lir/QoalaLir.cpp.inc"
