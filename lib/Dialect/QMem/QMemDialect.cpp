#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/Types.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Transforms/InliningUtils.h"
#include "llvm/ADT/TypeSwitch.h"

#include "Dialect/QMem/QMem.h"
#include "Dialect/QMem/QMemDialect.h"

// important! otherwise the source code in this inc file is not linked into the
// lib
#include "Dialect/QMem/QMemDialect.cpp.inc"

using namespace mlir;
using namespace qoala::dialects::qmem;

void QMemDialect::initialize() {
    addOperations<
#define GET_OP_LIST
#include "Dialect/QMem/QMem.cpp.inc"
        >();

    addTypes<
#define GET_TYPEDEF_LIST
#include "Dialect/QMem/QMemTypes.cpp.inc"
        >();
}
