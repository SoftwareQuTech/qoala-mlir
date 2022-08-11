#ifndef QOALA_HIR_DIALECT_TD
#define QOALA_HIR_DIALECT_TD

include "mlir/IR/DialectBase.td"

def QoalaHir_Dialect : Dialect {
  let name = "qoala_hir";

  let cppNamespace = "::mlir::qoala_hir";
}

#endif // QOALA_HIR_DIALECT_TD