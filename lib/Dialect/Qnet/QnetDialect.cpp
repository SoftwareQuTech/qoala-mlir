#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/Types.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Transforms/InliningUtils.h"
#include "llvm/ADT/TypeSwitch.h"

#include "Dialect/Qnet/Qnet.h"
#include "Dialect/Qnet/QnetDialect.h"

#include <iostream>
#include <execinfo.h>

// important! otherwise the source code in this inc file is not linked into the
// lib
#include "Dialect/Qnet/QnetDialect.cpp.inc"

using namespace mlir;
using namespace qoala::dialects::qnet;

void QnetDialect::initialize() {
  registerTypes();
  addOperations<
#define GET_OP_LIST
#include "Dialect/Qnet/Qnet.cpp.inc"
      >();
}
