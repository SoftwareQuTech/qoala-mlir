#ifndef NETQASM_DIALECT_TD
#define NETQASM_DIALECT_TD

include "mlir/IR/DialectBase.td"

def NetQASM_Dialect : Dialect {
    let name = "netqasm";

    let cppNamespace = "qoala::dialects::netqasm";
    let extraClassDeclaration = [{
        /**
         * Checks whether the given operation belongs to one of the allowed NetQASM dialects.
         * The list of allowed dialects is defined on "Dialect/NetQASM/NetQASM.h".
         * @param operation The operation to test
         * @return `true` if the operation belongs to one of the NetQASM allowed dialects.
         *         `false` otherwise.
         */
        static bool opIsNotFromAllowedDialects(mlir::Operation &operation);

        /**
         * Gets a string with the list of allowed dialects by NetQASM. The returned string
         * is useful for error reporting purposes.
         * The list of allowed dialects is defined on "Dialect/NetQASM/NetQASM.h".
         * @return A string with the dialect names that NetQASM defines as allowed.
         */
        static std::string getAllowedDialectNames();
    }];
}

#endif // NETQASM_DIALECT_TD