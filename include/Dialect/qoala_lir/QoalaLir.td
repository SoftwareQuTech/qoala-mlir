#ifndef QOALA_LIR_TD
#define QOALA_LIR_TD

// This file is the one that is used as input to mlir-tblgen.
// It should include the other .td files (including Dialect, Types, etc).

include "mlir/IR/OpBase.td"

include "Dialect/qoala_lir/QoalaLirDialect.td"
include "Dialect/qoala_lir/QoalaLirOps.td"

#endif // QOALA_LIR_TD
