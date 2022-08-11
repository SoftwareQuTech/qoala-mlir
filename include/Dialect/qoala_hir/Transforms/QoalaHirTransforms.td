#ifndef QOALA_HIR_TRANSFORMS_TD
#define QOALA_HIR_TRANSFORMS_TD

include "mlir/IR/PatternBase.td"

include "Dialect/qoala_hir/QoalaHirOps.td"

// def ReshapeReshapeOptPattern : Pat<(ReshapeOp(ReshapeOp $arg)), (ReshapeOp $arg)>;

#endif // QOALA_HIR_TRANSFORMS_TD