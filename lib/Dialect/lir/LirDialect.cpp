#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/Types.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Transforms/InliningUtils.h"
#include "llvm/ADT/TypeSwitch.h"

#include "Dialect/lir/LirDialect.h"
#include "Dialect/lir/Lir.h"

// important! otherwise the source code in this inc file is not linked into the lib
#include "Dialect/lir/LirDialect.cpp.inc"

using namespace mlir;
using namespace mlir::lir;

void LirDialect::initialize()
{
    // addOperations<ConstantOp>();

    addOperations<
#define GET_OP_LIST
#include "Dialect/lir/Lir.cpp.inc"
        >();

    addTypes<
#define GET_TYPEDEF_LIST
#include "Dialect/lir/LirTypes.cpp.inc"
        >();
}
