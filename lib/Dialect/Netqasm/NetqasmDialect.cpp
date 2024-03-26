#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/Types.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Transforms/InliningUtils.h"
#include "llvm/ADT/TypeSwitch.h"

#include "Dialect/Netqasm/Netqasm.h"
#include "Dialect/Netqasm/NetqasmDialect.h"

// important! otherwise the source code in this inc file is not linked into the
// lib
#include "Dialect/Netqasm/NetqasmDialect.cpp.inc"

using namespace mlir;
using namespace qoala::dialects::netqasm;

void NetqasmDialect::initialize() {
    addOperations<
#define GET_OP_LIST
#include "Dialect/Netqasm/Netqasm.cpp.inc"
        >();
}
