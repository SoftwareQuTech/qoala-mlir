#ifndef LIR_DIALECT_TD
#define LIR_DIALECT_TD

include "mlir/IR/DialectBase.td"

def Lir_Dialect : Dialect {
  let name = "lir";

  let cppNamespace = "::mlir::lir";

  // If 1, don't need to manually provide `parseType` and `printType` methods
  // in dialect source (cpp) file
  let useDefaultTypePrinterParser = 1;
}

#endif // LIR_DIALECT_TD