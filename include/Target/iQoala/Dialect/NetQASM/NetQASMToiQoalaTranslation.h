#ifndef NETQASMTOIQOALATRANSLATION_H
#define NETQASMTOIQOALATRANSLATION_H

#include "Analysis/Helpers/Helpers.h"
#include "Target/iQoala/ModuleTranslation.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/DialectRegistry.h"

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
