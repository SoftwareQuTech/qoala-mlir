#ifndef LIRCOMMON_LIRCOMMONTYPES_H
#define LIRCOMMON_LIRCOMMONTYPES_H

#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/TypeSupport.h"
#include "mlir/IR/Types.h"

#define GET_TYPEDEF_CLASSES
#include "Dialect/lircommon/LirCommonOpsTypes.h.inc"

#endif // LIRCOMMON_LIRCOMMONTYPES_H