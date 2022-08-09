#ifndef HIR_HIROPS_H
#define HIR_HIROPS_H

#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

#include "HirTypes.h"

#define GET_OP_CLASSES
#include "Dialect/hir/HirOps.h.inc"

#endif // HIR_HIROPS_H