#ifndef HIR_TRANSFORMS_REWRITES
#define HIR_TRANSFORMS_REWRITES

include "mlir/IR/OpBase.td"
include "mlir/Pass/PassBase.td"
include "mlir/Dialect/StandardOps/IR/Ops.td"
include "Dialect/hir/HirOps.td"

// def : Pat<(RotXOp(RotXOp $q)), (RotYOp $q)>;
def : Pat<(RotXOp $q, $c), (RotYOp $q, $c)>;

#endif // HIR_TRANSFORMS_REWRITES