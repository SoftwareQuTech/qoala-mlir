#include "mlir/IR/DialectImplementation.h"
// #include "mlir/IR/Types.h"
// #include "mlir/Support/LLVM.h"
// #include "mlir/Support/LogicalResult.h"
// #include "mlir/Transforms/InliningUtils.h"
// #include "llvm/ADT/TypeSwitch.h"

#include "Dialect/QRemote/QRemote.h"
#include "Dialect/QRemote/QRemoteDialect.h"

// important! otherwise the source code in this inc file is not linked into the
// lib
#include "Dialect/QRemote/QRemoteDialect.cpp.inc"

using namespace mlir;
using namespace qoala::dialects::qremote;

void QRemoteDialect::initialize() {
    addOperations<
#define GET_OP_LIST
#include "Dialect/QRemote/QRemote.cpp.inc"
            >();

    addTypes<
#define GET_TYPEDEF_LIST
#include "Dialect/QRemote/QRemoteTypes.cpp.inc"
            >();
}
