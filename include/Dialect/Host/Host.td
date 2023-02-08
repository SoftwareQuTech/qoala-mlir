#ifndef HOST_TD
#define HOST_TD

// This file is the one that is used as input to mlir-tblgen.
// It should include the other .td files (including Dialect, Types, etc).

include "mlir/IR/OpBase.td"

include "Dialect/Host/HostDialect.td"
include "Dialect/Host/HostTypes.td"
include "Dialect/Host/HostOps.td"

#endif // HOST_TD
