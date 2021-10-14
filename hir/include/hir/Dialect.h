#ifndef DIALECT_H_
#define DIALECT_H_

#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Dialect.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

#include "hir/Dialect.h.inc"

#define GET_OP_CLASSES
#include "hir/Ops.h.inc"

#endif // DIALECT_H_
