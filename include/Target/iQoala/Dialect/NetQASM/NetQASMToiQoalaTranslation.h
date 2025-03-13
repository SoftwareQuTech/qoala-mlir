#ifndef NETQASMTOIQOALATRANSLATION_H
#define NETQASMTOIQOALATRANSLATION_H

#include "Target/iQoala/ModuleTranslation.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/IR/Operation.h"

#include "Dialect/NetQASM/NetQASM.h"

namespace qoala::translate {
    class NetQASMToiQoalaTranslation : public QoalaTranslationDialectInterface {
    public:
        using QoalaTranslationDialectInterface::QoalaTranslationDialectInterface;

        mlir::LogicalResult convertOperation(mlir::Operation *op, ModuleTranslation *moduleTranslation) const final;

        static void registerInto(mlir::DialectRegistry &registry) {
            registeriQoalaTranslation<dialects::netqasm::NetQASMDialect, NetQASMToiQoalaTranslation>(registry);
        }
    };
}
#endif //NETQASMTOIQOALATRANSLATION_H
