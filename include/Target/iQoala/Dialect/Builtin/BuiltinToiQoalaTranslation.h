#ifndef BUILTINTOIQOALATRANSLATION_H
#define BUILTINTOIQOALATRANSLATION_H

#include "Analysis/Helpers/Helpers.h"
#include "Target/iQoala/ModuleTranslation.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/DialectRegistry.h"

#include "mlir/IR/BuiltinDialect.h"

namespace qoala::translate {
    class BuiltinToiQoalaTranslation : public QoalaTranslationDialectInterface {
    public:
        using QoalaTranslationDialectInterface::QoalaTranslationDialectInterface;

        mlir::LogicalResult convertOperation(mlir::Operation *op, ModuleTranslation &moduleTranslation) const final;

        static void registerInto(mlir::DialectRegistry &registry) {
            registeriQoalaTranslation<mlir::BuiltinDialect, BuiltinToiQoalaTranslation>(registry);
        }
    };
}


#endif //BUILTINTOIQOALATRANSLATION_H
