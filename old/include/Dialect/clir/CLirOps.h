#ifndef CLIR_CLIROPS_H
#define CLIR_CLIROPS_H

#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

#include "CLirTypes.h"

#define GET_OP_CLASSES
#include "Dialect/clir/CLirOps.h.inc"

#endif // CLIR_CLIROPS_H