#ifndef QREMOTE_TD
#define QREMOTE_TD

// This file is the one that is used as input to mlir-tblgen.
// It should include the other .td files (including Dialect, Types, etc).

include "mlir/IR/OpBase.td"

include "Dialect/QRemote/QRemoteDialect.td"
include "Dialect/QRemote/QRemoteOps.td"

#endif // QREMOTE_TD
