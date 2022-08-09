#ifndef IQOALA_TD
#define IQOALA_TD

// This file is the one that is used as input to mlir-tblgen.
// It should include the other .td files (including Dialect, Types, etc).

include "mlir/IR/OpBase.td"

include "Dialect/iqoala/iQoalaDialect.td"
include "Dialect/iqoala/iQoalaOps.td"

#endif // IQOALA_TD
