#ifndef CLIR_CLIRTYPES_H
#define CLIR_CLIRTYPES_H

#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/TypeSupport.h"
#include "mlir/IR/Types.h"

#define GET_TYPEDEF_CLASSES
#include "Dialect/clir/CLirOpsTypes.h.inc"

#endif // CLIR_CLIRTYPES_H