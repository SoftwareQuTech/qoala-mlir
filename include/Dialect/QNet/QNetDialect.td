#ifndef QNET_DIALECT_TD
#define QNET_DIALECT_TD

include "mlir/IR/DialectBase.td"

def QNet_Dialect : Dialect {
  let name = "qnet";

  let cppNamespace = "qoala::dialects::qnet";

  // If 1, don't need to manually provide `parseType` and `printType` methods
  // in dialect source (cpp) file
  let useDefaultTypePrinterParser = 1;
  let extraClassDeclaration = [{
    void registerTypes();
  }];
}

#endif // QNET_DIALECT_TD