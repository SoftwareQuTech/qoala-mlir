#ifndef HIR_HIRTYPES_H
#define HIR_HIRTYPES_H

#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/TypeSupport.h"
#include "mlir/IR/Types.h"

#define GET_TYPEDEF_CLASSES
#include "Dialect/hir/HirOpsTypes.h.inc"

#endif // HIR_HIRTYPES_H