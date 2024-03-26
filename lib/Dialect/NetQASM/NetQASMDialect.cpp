#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/Types.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Transforms/InliningUtils.h"
#include "llvm/ADT/TypeSwitch.h"

#include "Dialect/NetQASM/NetQASM.h"
#include "Dialect/NetQASM/NetQASMDialect.h"

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
