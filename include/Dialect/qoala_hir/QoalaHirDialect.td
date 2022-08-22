#ifndef QOALA_HIR_DIALECT_TD
#define QOALA_HIR_DIALECT_TD

include "mlir/IR/DialectBase.td"

def QoalaHir_Dialect : Dialect {
  let name = "qoala_hir";

  let cppNamespace = "::mlir::qoala_hir";

  // Needed so that "parseType" and "printType" functions are generated
  let useDefaultTypePrinterParser = 1;
}

#endif // QOALA_HIR_DIALECT_TD