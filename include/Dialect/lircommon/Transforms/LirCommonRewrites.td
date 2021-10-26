#ifndef LIRCOMMON_LIRCOMMON_REWRITES
#define LIRCOMMON_LIRCOMMON_REWRITES

include "mlir/IR/OpBase.td"
include "mlir/Pass/PassBase.td"
include "mlir/Dialect/StandardOps/IR/Ops.td"
include "Dialect/lircommon/LirCommonOps.td"

// def : Pat<(GateXOp $q, $c), (GateYOp $q, $c)>;
// def : Pat<(GateXOp (GateXOp $q, $c1), $c2), (GateXOp $q, (AddCValueOnQuanOp $c1, $c2))>;
// def : Pat<(GateHOp (GateHOp (LirCommon_AllocateOp:$q))), (LirCommon_AllocateOp)>;
// def : Pat<(GateHOp (GateHOp $q)), (replaceWithValue $q)>;

#endif // LIRCOMMON_LIRCOMMON_REWRITES