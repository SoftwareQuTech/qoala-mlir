#ifndef QOALAHIR_QOALAHIROPS_H
#define QOALAHIR_QOALAHIROPS_H

#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/Interfaces/InferTypeOpInterface.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

#define GET_OP_CLASSES
#include "Dialect/hir/HirOps.h.inc"

#endif // QOALAHIR_QOALAHIROPS_H
