#ifndef HIR_TD
#define HIR_TD

// This file is the one that is used as input to mlir-tblgen.
// It should include the other .td files (including Dialect, Types, etc).

include "mlir/IR/OpBase.td"

include "Dialect/hir/HirDialect.td"
include "Dialect/hir/HirTypes.td"
include "Dialect/hir/HirOps.td"

#endif // HIR_TD
