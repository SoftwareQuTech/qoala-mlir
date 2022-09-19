#ifndef QOALA_LIR_M_TRANSFORMS_TD
#define QOALA_LIR_M_TRANSFORMS_TD

include "mlir/IR/PatternBase.td"

include "Dialect/qoala_lir_m/QoalaLirMOps.td"

// def ReshapeReshapeOptPattern : Pat<(ReshapeOp(ReshapeOp $arg)), (ReshapeOp $arg)>;

#endif // QOALA_LIR_M_TRANSFORMS_TD