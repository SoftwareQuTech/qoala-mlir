#ifndef LIR_LIR_REWRITES
#define LIR_LIR_REWRITES

include "mlir/IR/OpBase.td"
include "mlir/Pass/PassBase.td"
include "mlir/Dialect/StandardOps/IR/Ops.td"
include "Dialect/lir/LirOps.td"

// def : Pat<(RotXOp $q, $c), (RotYOp $q, $c)>;
// def : Pat<(RotXOp (RotXOp $q, $c1), $c2), (RotXOp $q, (AddCValueQOp $c1, $c2))>;
// def : Pat<(GateHOp (GateHOp (Lir_AllocateOp:$q))), (Lir_AllocateOp)>;
// def : Pat<(GateHOp (GateHOp $q)), (replaceWithValue $q)>;

#endif // LIR_LIR_REWRITES