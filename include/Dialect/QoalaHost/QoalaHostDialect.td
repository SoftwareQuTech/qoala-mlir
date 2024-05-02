#ifndef QOALAHOST_DIALECT_TD
#define QOALAHOST_DIALECT_TD

include "mlir/IR/DialectBase.td"

def QoalaHost_Dialect : Dialect {
    let name = "qoalahost";

    let cppNamespace = "qoala::dialects::qoalahost";
    let extraClassDeclaration = [{
        /**
         * Checks whether the given operation belongs to one of the allowed QoalaHost dialects.
         * The list of allowed dialects is defined on "Dialect/QoalaHost/QoalaHost.h".
         * @param operation The operation to test
         * @return `true` if the operation belongs to one of the QoalaHost allowed dialects.
         *         `false` otherwise.
         */
        static bool opIsNotFromAllowedDialects(mlir::Operation &operation);

        /**
         * Gets a string with the list of allowed dialects by QoalaHost. The returned string
         * is useful for error reporting purposes.
         * The list of allowed dialects is defined on "Dialect/QoalaHost/QoalaHost.h".
         * @return A string with the dialect names that QoalaHost defines as allowed.
         */
        static std::string getAllowedDialectNames();
    }];
}

#endif // QOALAHOST_DIALECT_TD