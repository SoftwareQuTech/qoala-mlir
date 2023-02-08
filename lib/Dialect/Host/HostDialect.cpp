#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/Types.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Transforms/InliningUtils.h"
#include "llvm/ADT/TypeSwitch.h"

#include "Dialect/Host/HostDialect.h"
#include "Dialect/Host/Host.h"

// important! otherwise the source code in this inc file is not linked into the lib
#include "Dialect/Host/HostDialect.cpp.inc"

using namespace mlir;
using namespace mlir::host;

void HostDialect::initialize()
{
    // addOperations<ConstantOp>();

    addOperations<
#define GET_OP_LIST
#include "Dialect/Host/Host.cpp.inc"
        >();

    addTypes<
#define GET_TYPEDEF_LIST
#include "Dialect/Host/HostTypes.cpp.inc"
        >();
}
