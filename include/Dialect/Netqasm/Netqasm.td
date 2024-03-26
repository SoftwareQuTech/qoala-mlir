#ifndef NETQASM_TD
#define NETQASM_TD

// This file is the one that is used as input to mlir-tblgen.
// It should include the other .td files (including Dialect, Types, etc).

include "mlir/IR/OpBase.td"

include "Dialect/netqasm/NetqasmDialect.td"
include "Dialect/netqasm/NetqasmTypes.td"
include "Dialect/netqasm/NetqasmOps.td"

#endif // NETQASM_TD
