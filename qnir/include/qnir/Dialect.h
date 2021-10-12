#ifndef DIALECT_H_
#define DIALECT_H_

#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Dialect.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

#include "qnir/Dialect.h.inc"

#define GET_OP_CLASSES
#include "qnir/Ops.h.inc"

#endif // DIALECT_H_
