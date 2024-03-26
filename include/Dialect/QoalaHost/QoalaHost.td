#ifndef QOALAHOST_TD
#define QOALAHOST_TD

// This file is the one that is used as input to mlir-tblgen.
// It should include the other .td files (including Dialect, Types, etc).

include "mlir/IR/OpBase.td"

include "Dialect/QoalaHost/QoalaHostDialect.td"
include "Dialect/QoalaHost/QoalaHostOps.td"

#endif // QOALAHOST_TD
