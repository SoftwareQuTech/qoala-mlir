#ifndef IQOALA_DIALECT_TD
#define IQOALA_DIALECT_TD

include "mlir/IR/DialectBase.td"

def iQoala_Dialect : Dialect {
  let name = "iqoala";

  let cppNamespace = "::mlir::iqoala";
}

#endif // IQOALA_DIALECT_TD