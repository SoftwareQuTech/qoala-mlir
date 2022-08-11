#ifndef QOALAtest_TD
#define QOALAtest_TD

// This file is the one that is used as input to mlir-tblgen.
// It should include the other .td files (including Dialect, Types, etc).

include "mlir/IR/OpBase.td"

include "Dialect/qoala_test/QoalaTestDialect.td"
include "Dialect/qoala_test/QoalaTestOps.td"

#endif // QOALAtest_TD
