#ifndef LIR_LIROPS_H
#define LIR_LIROPS_H

#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

#include "LirTypes.h"

#define GET_OP_CLASSES
#include "Dialect/lir/LirOps.h.inc"

#endif // LIR_LIROPS_H