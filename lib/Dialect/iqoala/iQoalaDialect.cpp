#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/Types.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Transforms/InliningUtils.h"
#include "llvm/ADT/TypeSwitch.h"

#include "Dialect/iqoala/iQoalaDialect.h"
#include "Dialect/iqoala/iQoala.h"

// important! otherwise the source code in this inc file is not linked into the lib
#include "Dialect/iqoala/iQoalaDialect.cpp.inc"

using namespace mlir;
using namespace mlir::iqoala;

void iQoalaDialect::initialize()
{
    // addOperations<ConstantOp>();

    addOperations<
#define GET_OP_LIST
#include "Dialect/iqoala/iQoala.cpp.inc"
        >();

    //     addTypes<
    // #define GET_TYPEDEF_LIST
    // #include "Dialect/iqoala/iQoalaTypes.cpp.inc"
    //         >();
}
