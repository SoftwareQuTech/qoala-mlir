#ifndef LIR_TD
#define LIR_TD

// This file is the one that is used as input to mlir-tblgen.
// It should include the other .td files (including Dialect, Types, etc).

include "mlir/IR/OpBase.td"

include "Dialect/lir/LirDialect.td"
include "Dialect/lir/LirTypes.td"
include "Dialect/lir/LirOps.td"

#endif // LIR_TD
