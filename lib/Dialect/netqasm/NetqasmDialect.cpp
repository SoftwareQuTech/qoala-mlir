#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/Types.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Transforms/InliningUtils.h"
#include "llvm/ADT/TypeSwitch.h"

#include "Dialect/netqasm/Netqasm.h"
#include "Dialect/netqasm/NetqasmDialect.h"

// important! otherwise the source code in this inc file is not linked into the
// lib
#include "Dialect/netqasm/NetqasmDialect.cpp.inc"

using namespace mlir;
using namespace mlir::netqasm;

void NetqasmDialect::initialize() {
  // addOperations<ConstantOp>();

  addOperations<
#define GET_OP_LIST
#include "Dialect/netqasm/Netqasm.cpp.inc"
      >();

  addTypes<
#define GET_TYPEDEF_LIST
#include "Dialect/netqasm/NetqasmTypes.cpp.inc"
      >();
}
