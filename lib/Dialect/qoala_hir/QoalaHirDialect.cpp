#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/Types.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Transforms/InliningUtils.h"
#include "llvm/ADT/TypeSwitch.h"

#include "Dialect/qoala_hir/QoalaHirDialect.h"
#include "Dialect/qoala_hir/QoalaHir.h"

// important! otherwise the source code in this inc file is not linked into the lib
#include "Dialect/qoala_hir/QoalaHirDialect.cpp.inc"

using namespace mlir;
using namespace mlir::qoala_hir;

void QoalaHirDialect::initialize()
{
    // addOperations<ConstantOp>();

    addOperations<
#define GET_OP_LIST
#include "Dialect/qoala_hir/QoalaHir.cpp.inc"
        >();

    //     addTypes<
    // #define GET_TYPEDEF_LIST
    // #include "Dialect/qoala_hir/QoalaHirTypes.cpp.inc"
    //         >();
}
