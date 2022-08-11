#ifndef QOALAtest_DIALECT_TD
#define QOALAtest_DIALECT_TD

include "mlir/IR/DialectBase.td"

def QoalaTest_Dialect : Dialect {
  let name = "qoala_test";

  let cppNamespace = "::mlir::qoala_test";

  let emitAccessorPrefix = kEmitAccessorPrefix_Prefixed;
}

#endif // QOALAtest_DIALECT_TD