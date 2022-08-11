#ifndef QOALA_HIR_H
#define QOALA_HIR_H

#include "mlir/IR/OpDefinition.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/IR/BuiltinAttributes.h"

// Needed for [NoSideEffect] in the .td file.
#include "mlir/Interfaces/SideEffectInterfaces.h"

#define GET_OP_CLASSES
#include "Dialect/qoala_hir/QoalaHir.h.inc"

using namespace mlir;

#endif // QOALA_HIR_H