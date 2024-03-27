#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/Types.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Transforms/InliningUtils.h"
#include "llvm/ADT/TypeSwitch.h"

#include "Dialect/QNet/QNet.h"
#include "Dialect/QNet/QNetDialect.h"

#include <execinfo.h>
#include <iostream>

// important! otherwise the source code in this inc file is not linked into the
// lib
#include "Dialect/QNet/QNetDialect.cpp.inc"

using namespace mlir;
using namespace qoala::dialects::qnet;

void QNetDialect::initialize() {
    registerTypes();
    addOperations<
#define GET_OP_LIST
#include "Dialect/QNet/QNet.cpp.inc"
        >();
}
