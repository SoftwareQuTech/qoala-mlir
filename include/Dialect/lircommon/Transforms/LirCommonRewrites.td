#ifndef LIRCOMMON_LIRCOMMON_REWRITES
#define LIRCOMMON_LIRCOMMON_REWRITES

include "mlir/IR/OpBase.td"
include "mlir/Pass/PassBase.td"
include "mlir/Dialect/StandardOps/IR/Ops.td"
include "Dialect/lircommon/LirCommonOps.td"

def : Pat<(GateXOp $q, $c), (GateYOp $q, $c)>;

#endif // LIRCOMMON_LIRCOMMON_REWRITES