#ifndef QOALAHOST_DIALECT_TD
#define QOALAHOST_DIALECT_TD

include "mlir/IR/DialectBase.td"

def QoalaHost_Dialect : Dialect {
  let name = "qoalahost";

  let cppNamespace = "qoala::dialects::qoalahost";
}

#endif // QOALAHOST_DIALECT_TD