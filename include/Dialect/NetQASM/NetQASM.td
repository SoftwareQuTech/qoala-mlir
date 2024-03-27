#ifndef NETQASM_TD
#define NETQASM_TD

// This file is the one that is used as input to mlir-tblgen.
// It should include the other .td files (including Dialect, Types, etc).

include "mlir/IR/OpBase.td"

include "Dialect/NetQASM/NetQASMDialect.td"
include "Dialect/NetQASM/NetQASMOps.td"

#endif // NETQASM_TD
