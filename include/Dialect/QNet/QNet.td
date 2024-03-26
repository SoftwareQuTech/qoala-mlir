#ifndef QNET_TD
#define QNET_TD

// This file is the one that is used as input to mlir-tblgen.
// It should include the other .td files (including Dialect, Types, etc).

include "mlir/IR/OpBase.td"

include "Dialect/QNet/QNetDialect.td"
include "Dialect/QNet/QNetTypes.td"
include "Dialect/QNet/QNetOps.td"

#endif // QNET_TD
