#include "mlir/IR/DialectImplementation.h"

#include "Dialect/NetQASM/NetQASM.h"
#include "Dialect/NetQASM/NetQASMDialect.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Dialect/QoalaHost/QoalaHostDialect.h"

// important! otherwise the source code in this inc file is not linked into the
// lib
#include "Dialect/NetQASM/NetQASMDialect.cpp.inc"

using namespace mlir;
using namespace qoala::dialects::netqasm;

void NetQASMDialect::initialize() {
    addOperations<
#define GET_OP_LIST
#include "Dialect/NetQASM/NetQASM.cpp.inc"
            >();
}
