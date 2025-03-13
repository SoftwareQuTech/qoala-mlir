#ifndef QOALAHOSTTOIQOALATRANSLATION_H
#define QOALAHOSTTOIQOALATRANSLATION_H

#include "Target/iQoala/ModuleTranslation.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/IR/Operation.h"

#include "Dialect/QoalaHost/QoalaHost.h"

namespace qoala::translate {
    class QoalaHostToiQoalaTranslation : public QoalaTranslationDialectInterface {
    public:
        using QoalaTranslationDialectInterface::QoalaTranslationDialectInterface;

        mlir::LogicalResult convertOperation(mlir::Operation *op, ModuleTranslation *moduleTranslation) const final;

        static void registerInto(mlir::DialectRegistry &registry) {
            registeriQoalaTranslation<dialects::qoalahost::QoalaHostDialect, QoalaHostToiQoalaTranslation>(registry);
        }
    };
}

#endif //QOALAHOSTTOIQOALATRANSLATION_H
