#ifndef QOALA_LIR_M_TD
#define QOALA_LIR_M_TD

// This file is the one that is used as input to mlir-tblgen.
// It should include the other .td files (including Dialect, Types, etc).

include "mlir/IR/OpBase.td"

include "Dialect/qoala_lir_m/QoalaLirMDialect.td"
include "Dialect/qoala_lir_m/QoalaLirMTypes.td"
include "Dialect/qoala_lir_m/QoalaLirMOps.td"

#endif // QOALA_LIR_M_TD
