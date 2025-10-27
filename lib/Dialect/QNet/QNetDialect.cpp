#include "mlir/IR/DialectImplementation.h"

#include "Dialect/QNet/QNet.h"
#include "Dialect/QNet/QNetDialect.h"

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
