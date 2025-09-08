#ifndef BUILTINTOIQOALATRANSLATION_H
#define BUILTINTOIQOALATRANSLATION_H

#include "Target/iQoala/ModuleTranslation.h"
#include "mlir/IR/BuiltinDialect.h"
#include "mlir/Support/LogicalResult.h"

namespace qoala::translate {
    class BuiltinToiQoalaTranslation : public QoalaTranslationDialectInterface {
    public:
        using QoalaTranslationDialectInterface::QoalaTranslationDialectInterface;

        mlir::LogicalResult convertOperation(mlir::Operation *op, ModuleTranslation *moduleTranslation) const final;

        static void registerInto(mlir::DialectRegistry &registry) {
            registeriQoalaTranslation<mlir::BuiltinDialect, BuiltinToiQoalaTranslation>(registry);
        }
    };
} // namespace qoala::translate

#endif // BUILTINTOIQOALATRANSLATION_H
