#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/Types.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Transforms/InliningUtils.h"
#include "llvm/ADT/TypeSwitch.h"

#include "Dialect/Qmem/Qmem.h"
#include "Dialect/Qmem/QmemDialect.h"

// important! otherwise the source code in this inc file is not linked into the
// lib
#include "Dialect/Qmem/QmemDialect.cpp.inc"

using namespace mlir;
using namespace qoala::dialects::qmem;

void QmemDialect::initialize() {
    // addOperations<ConstantOp>();

    addOperations<
#define GET_OP_LIST
#include "Dialect/Qmem/Qmem.cpp.inc"
        >();

    addTypes<
#define GET_TYPEDEF_LIST
#include "Dialect/Qmem/QmemTypes.cpp.inc"
        >();
}
