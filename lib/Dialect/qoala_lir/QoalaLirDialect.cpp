#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/Types.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Transforms/InliningUtils.h"
#include "llvm/ADT/TypeSwitch.h"

#include "Dialect/qoala_lir/QoalaLirDialect.h"
#include "Dialect/qoala_lir/QoalaLir.h"

// important! otherwise the source code in this inc file is not linked into the lib
#include "Dialect/qoala_lir/QoalaLirDialect.cpp.inc"

using namespace mlir;
using namespace mlir::qoala_lir;

void QoalaLirDialect::initialize()
{
    // addOperations<ConstantOp>();

    addOperations<
#define GET_OP_LIST
#include "Dialect/qoala_lir/QoalaLir.cpp.inc"
        >();

    //     addTypes<
    // #define GET_TYPEDEF_LIST
    // #include "Dialect/qoala_lir/QoalaLirTypes.cpp.inc"
    //         >();
}
