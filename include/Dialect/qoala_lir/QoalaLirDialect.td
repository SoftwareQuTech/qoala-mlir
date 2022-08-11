#ifndef QOALA_LIR_DIALECT_TD
#define QOALA_LIR_DIALECT_TD

include "mlir/IR/DialectBase.td"

def QoalaLir_Dialect : Dialect {
  let name = "qoala_lir";

  let cppNamespace = "::mlir::qoala_lir";

  // let emitAccessorPrefix = kEmitAccessorPrefix_Prefixed;
}

#endif // QOALA_LIR_DIALECT_TD