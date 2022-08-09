#ifndef TOY_TOY_H
#define TOY_TOY_H

#include "mlir/IR/OpDefinition.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/IR/BuiltinAttributes.h"

#include "mlir/Interfaces/SideEffectInterfaces.h"

#define GET_OP_CLASSES
#include "Dialect/toy/Toy.h.inc"

using namespace mlir;

#endif // TOY_TOY_H