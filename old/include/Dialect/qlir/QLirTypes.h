#ifndef QLIR_QLIRTYPES_H
#define QLIR_QLIRTYPES_H

#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/TypeSupport.h"
#include "mlir/IR/Types.h"

#define GET_TYPEDEF_CLASSES
#include "Dialect/qlir/QLirOpsTypes.h.inc"

#endif // QLIR_QLIRTYPES_H