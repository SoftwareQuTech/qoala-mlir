#ifndef TOY_OPS
#define TOY_OPS

// This file is the one that is used as input to mlir-tblgen.
// It should include the other .td files (including Dialect, Types, etc).

include "mlir/IR/OpBase.td"

include "Dialect/toy/ToyDialect.td"

#endif // TOY_OPS
