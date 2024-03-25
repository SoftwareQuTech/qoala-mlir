#ifndef QNET_TD
#define QNET_TD

// This file is the one that is used as input to mlir-tblgen.
// It should include the other .td files (including Dialect, Types, etc).

include "mlir/IR/OpBase.td"

include "Dialect/Qnet/QnetDialect.td"
include "Dialect/Qnet/QnetTypes.td"
include "Dialect/Qnet/QnetOps.td"

#endif // QNET_TD
