#ifndef QLIR_QLIROPS_H
#define QLIR_QLIROPS_H

#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

#include "QLirTypes.h"

#define GET_OP_CLASSES
#include "Dialect/qlir/QLirOps.h.inc"

#endif // QLIR_QLIROPS_H