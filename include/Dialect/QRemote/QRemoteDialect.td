#ifndef QREMOTE_DIALECT_TD
#define QREMOTE_DIALECT_TD

include "mlir/IR/DialectBase.td"

def QRemote_Dialect : Dialect {
    let name = "qremote";

    let cppNamespace = "qoala::dialects::qremote";
}

#endif // QREMOTE_DIALECT_TD