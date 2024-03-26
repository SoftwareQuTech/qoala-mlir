#ifndef QMEM_TD
#define QMEM_TD

// This file is the one that is used as input to mlir-tblgen.
// It should include the other .td files (including Dialect, Types, etc).

include "mlir/IR/OpBase.td"

include "Dialect/Qmem/QmemDialect.td"
include "Dialect/Qmem/QmemOps.td"

#endif // QMEM_TD
