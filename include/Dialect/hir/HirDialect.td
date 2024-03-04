#ifndef HIR_DIALECT_TD
#define HIR_DIALECT_TD

include "mlir/IR/DialectBase.td"

def Hir_Dialect : Dialect {
  let name = "hir";

  let cppNamespace = "::mlir::hir";

  // If 1, don't need to manually provide `parseType` and `printType` methods
  // in dialect source (cpp) file
  let useDefaultTypePrinterParser = 1;
  let extraClassDeclaration = [{
    void registerTypes();
  }];
}

#endif // HIR_DIALECT_TD