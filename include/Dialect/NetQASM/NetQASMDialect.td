#ifndef NETQASM_DIALECT_TD
#define NETQASM_DIALECT_TD

include "mlir/IR/DialectBase.td"

def NetQASM_Dialect : Dialect {
    let name = "netqasm";

    let cppNamespace = "qoala::dialects::netqasm";
    let dependentDialects = ["qoala::dialects::qoalahost::QoalaHostDialect"];
}

#endif // NETQASM_DIALECT_TD