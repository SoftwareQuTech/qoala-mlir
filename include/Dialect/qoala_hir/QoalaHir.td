#ifndef QOALA_HIR_TD
#define QOALA_HIR_TD

// This file is the one that is used as input to mlir-tblgen.
// It should include the other .td files (including Dialect, Types, etc).

include "mlir/IR/OpBase.td"

include "Dialect/qoala_hir/QoalaHirDialect.td"
include "Dialect/qoala_hir/QoalaHirOps.td"

#endif // QOALA_HIR_TD
