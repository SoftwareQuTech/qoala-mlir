#ifndef QOALA_LIR_TRANSFORMS_TD
#define QOALA_LIR_TRANSFORMS_TD

include "mlir/IR/PatternBase.td"

include "Dialect/qoala_lir/QoalaLirOps.td"

// def ReshapeReshapeOptPattern : Pat<(ReshapeOp(ReshapeOp $arg)), (ReshapeOp $arg)>;

#endif // QOALA_LIR_TRANSFORMS_TD