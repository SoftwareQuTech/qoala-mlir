#ifndef NETQASM_DIALECT_TD
#define NETQASM_DIALECT_TD

include "mlir/IR/DialectBase.td"

def Netqasm_Dialect : Dialect {
  let name = "netqasm";

  let cppNamespace = "qoala::dialects::netqasm";
}

#endif // NETQASM_DIALECT_TD