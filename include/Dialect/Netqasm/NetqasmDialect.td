#ifndef NETQASM_DIALECT_TD
#define NETQASM_DIALECT_TD

include "mlir/IR/DialectBase.td"

def Netqasm_Dialect : Dialect {
  let name = "netqasm";

  let cppNamespace = "::mlir::netqasm";

  // If 1, don't need to manually provide `parseType` and `printType` methods
  // in dialect source (cpp) file
  let useDefaultTypePrinterParser = 1;
}

#endif // NETQASM_DIALECT_TD