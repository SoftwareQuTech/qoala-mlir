#ifndef IQOALA_H
#define IQOALA_H

#include "mlir/IR/OpDefinition.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/IR/BuiltinAttributes.h"

// Needed for [NoSideEffect] in the .td file.
#include "mlir/Interfaces/SideEffectInterfaces.h"

#define GET_OP_CLASSES
#include "Dialect/iqoala/iQoala.h.inc"

using namespace mlir;

#endif // IQOALA_H