#ifndef LIRCOMMON_LIRCOMMONOPS_H
#define LIRCOMMON_LIRCOMMONOPS_H

#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

#include "LirCommonTypes.h"

#define GET_OP_CLASSES
#include "Dialect/lircommon/LirCommonOps.h.inc"

#endif // LIRCOMMON_LIRCOMMONOPS_H