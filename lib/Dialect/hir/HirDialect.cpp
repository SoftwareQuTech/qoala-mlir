#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/Types.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Transforms/InliningUtils.h"
#include "llvm/ADT/TypeSwitch.h"

#include "Dialect/hir/Hir.h"
#include "Dialect/hir/HirDialect.h"

#include <iostream>
#include <execinfo.h>

// important! otherwise the source code in this inc file is not linked into the
// lib
#include "Dialect/hir/HirDialect.cpp.inc"

using namespace mlir;
using namespace mlir::hir;

void HirDialect::initialize() {
  registerTypes();
  addOperations<
#define GET_OP_LIST
#include "Dialect/hir/Hir.cpp.inc"
      >();
}
