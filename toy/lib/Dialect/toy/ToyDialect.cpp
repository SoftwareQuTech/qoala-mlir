#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/Types.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Transforms/InliningUtils.h"
#include "llvm/ADT/TypeSwitch.h"

#include "Dialect/toy/ToyDialect.h"
#include "Dialect/toy/Toy.h"

// important! otherwise the source code in this inc file is not linked into the lib
#include "Dialect/toy/ToyDialect.cpp.inc"

using namespace mlir;
using namespace mlir::toy;


void ToyDialect::initialize()
{
//     addOperations<
// #define GET_OP_LIST
// #include "Dialect/toy/Toy.cpp.inc"
//         >();
//     addTypes<
// #define GET_TYPEDEF_LIST
// #include "Dialect/toy/ToyTypes.cpp.inc"
//         >();
}
