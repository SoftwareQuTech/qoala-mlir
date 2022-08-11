#ifndef QOALA_LIR_H
#define QOALA_LIR_H

#include "mlir/IR/OpDefinition.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/IR/BuiltinAttributes.h"

// Needed for [FunctionOpInterface] in the .td file.
#include "mlir/IR/FunctionInterfaces.h"

// Needed for [NoSideEffect] in the .td file.
#include "mlir/Interfaces/SideEffectInterfaces.h"

// #include "mlir/IR/SymbolInterfaces.h.inc"
#include "mlir/Interfaces/CallInterfaces.h"
// #include "mlir/IR/OpAsmInterface.h.inc"

#define GET_OP_CLASSES
#include "Dialect/qoala_test/QoalaTest.h.inc"

using namespace mlir;

#endif // QOALA_LIR_H