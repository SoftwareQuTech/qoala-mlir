#ifndef LIR_LIRTYPES_H
#define LIR_LIRTYPES_H

#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/TypeSupport.h"
#include "mlir/IR/Types.h"

#define GET_TYPEDEF_CLASSES
#include "Dialect/lir/LirOpsTypes.h.inc"

#endif // LIR_LIRTYPES_H