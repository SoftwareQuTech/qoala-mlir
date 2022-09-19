#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/Types.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Transforms/InliningUtils.h"
#include "llvm/ADT/TypeSwitch.h"

#include "Dialect/qoala_lir_m/QoalaLirMDialect.h"
#include "Dialect/qoala_lir_m/QoalaLirM.h"

// important! otherwise the source code in this inc file is not linked into the lib
#include "Dialect/qoala_lir_m/QoalaLirMDialect.cpp.inc"

using namespace mlir;
using namespace mlir::qoala_lir_m;

void QoalaLirMDialect::initialize()
{
    // addOperations<ConstantOp>();

    addOperations<
#define GET_OP_LIST
#include "Dialect/qoala_lir_m/QoalaLirM.cpp.inc"
        >();

    addTypes<
#define GET_TYPEDEF_LIST
#include "Dialect/qoala_lir_m/QoalaLirMTypes.cpp.inc"
        >();
}
