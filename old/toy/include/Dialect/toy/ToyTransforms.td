#ifndef TOY_TRANSFORMS
#define TOY_TRANSFORMS

include "mlir/IR/PatternBase.td"

include "Dialect/toy/ToyOps.td"

def ReshapeReshapeOptPattern : Pat<(ReshapeOp(ReshapeOp $arg)), (ReshapeOp $arg)>;

#endif // TOY_TRANSFORMS