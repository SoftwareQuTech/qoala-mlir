// NOTE: clang-format is disabled for the following includes.
// These must remain in this specific order, changing it breaks the build.
// clang-format off
#include "Dialect/NetQASM/NetQASM.h"
#include "Dialect/NetQASM/NetQASMDialect.h"
// clang-format on
#include "Dialect/QoalaHost/QoalaHost.h"

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
