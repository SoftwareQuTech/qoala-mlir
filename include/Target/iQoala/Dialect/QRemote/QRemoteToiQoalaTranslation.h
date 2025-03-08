#ifndef QREMOTETOIQOALATRANSLATION_H
#define QREMOTETOIQOALATRANSLATION_H

#include "Analysis/Helpers/Helpers.h"
#include "Target/iQoala/ModuleTranslation.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/DialectRegistry.h"

#include "Dialect/QRemote/QRemote.h"

namespace qoala::translate {
    class QRemoteToiQoalaTranslation : public QoalaTranslationDialectInterface {
    public:
        using QoalaTranslationDialectInterface::QoalaTranslationDialectInterface;

        mlir::LogicalResult convertOperation(mlir::Operation *op, ModuleTranslation *moduleTranslation) const final;

        static void registerInto(mlir::DialectRegistry &registry) {
            registeriQoalaTranslation<dialects::qremote::QRemoteDialect, QRemoteToiQoalaTranslation>(registry);
        }
    };
}

#endif //QREMOTETOIQOALATRANSLATION_H
