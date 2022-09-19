#ifndef TOY_TD
#define TOY_TD

// This file is the one that is used as input to mlir-tblgen.
// It should include the other .td files (including Dialect, Types, etc).

include "mlir/IR/OpBase.td"

include "Dialect/toy/ToyDialect.td"
include "Dialect/toy/ToyOps.td"

#endif // TOY_TD
