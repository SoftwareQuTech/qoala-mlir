#include "mlir/IR/DialectImplementation.h"

#include "Dialect/NetQASM/NetQASM.h"
#include "Dialect/NetQASM/NetQASMDialect.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Dialect/QoalaHost/QoalaHostDialect.h"

// important! otherwise the source code in this inc file is not linked into the
// lib
#include "Dialect/QoalaHost/QoalaHostDialect.cpp.inc"

using namespace mlir;
using namespace qoala::dialects::qoalahost;

void QoalaHostDialect::initialize() {
    addOperations<
#define GET_OP_LIST
#include "Dialect/QoalaHost/QoalaHost.cpp.inc"
        >();

    addTypes<
#define GET_TYPEDEF_LIST
#include "Dialect/QoalaHost/QoalaHostTypes.cpp.inc"
        >();
}
