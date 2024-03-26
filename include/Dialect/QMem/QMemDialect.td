#ifndef QMEM_DIALECT_TD
#define QMEM_DIALECT_TD

include "mlir/IR/DialectBase.td"

def QMem_Dialect : Dialect {
  let name = "qmem";

  let cppNamespace = "qoala::dialects::qmem";
}

#endif // QMEM_DIALECT_TD