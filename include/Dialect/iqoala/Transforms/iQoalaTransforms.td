#ifndef IQOALA_TRANSFORMS_TD
#define IQOALA_TRANSFORMS_TD

include "mlir/IR/PatternBase.td"

include "Dialect/iqoala/iQoalaOps.td"

def ReshapeReshapeOptPattern : Pat<(ReshapeOp(ReshapeOp $arg)), (ReshapeOp $arg)>;

#endif // IQOALA_TRANSFORMS_TD