#ifndef CFTOIQOALATRANSLATION_H
#define CFTOIQOALATRANSLATION_H

#include "Target/iQoala/ModuleTranslation.h"
#include "mlir/Support/LogicalResult.h"

#include "mlir/Dialect/ControlFlow/IR/ControlFlow.h"

namespace qoala::translate {
    class ControlFlowToiQoalaTranslation : public QoalaTranslationDialectInterface {
    public:
        using QoalaTranslationDialectInterface::QoalaTranslationDialectInterface;

        mlir::LogicalResult convertOperation(mlir::Operation *op, ModuleTranslation *moduleTranslation) const final;

        static void registerInto(mlir::DialectRegistry &registry) {
            registeriQoalaTranslation<mlir::cf::ControlFlowDialect, ControlFlowToiQoalaTranslation>(registry);
        }
    };
} // namespace qoala::translate
#endif // CFTOIQOALATRANSLATION_H
