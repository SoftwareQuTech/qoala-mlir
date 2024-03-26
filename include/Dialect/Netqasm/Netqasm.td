#ifndef NETQASM_TD
#define NETQASM_TD

// This file is the one that is used as input to mlir-tblgen.
// It should include the other .td files (including Dialect, Types, etc).

include "mlir/IR/OpBase.td"

include "Dialect/Netqasm/NetqasmDialect.td"
include "Dialect/Netqasm/NetqasmOps.td"

#endif // NETQASM_TD
