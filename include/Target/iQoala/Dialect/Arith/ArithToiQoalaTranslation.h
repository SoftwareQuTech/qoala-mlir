#ifndef ARITHTOIQOALATRANSLATION_H
#define ARITHTOIQOALATRANSLATION_H

#include "Target/iQoala/ModuleTranslation.h"
#include "mlir/Support/LogicalResult.h"

#include "mlir/Dialect/Arith/IR/Arith.h"

namespace qoala::translate {
    class ArithToiQoalaTranslation : public QoalaTranslationDialectInterface {
    public:
        using QoalaTranslationDialectInterface::QoalaTranslationDialectInterface;

        mlir::LogicalResult convertOperation(mlir::Operation *op, ModuleTranslation *moduleTranslation) const final;

        static void registerInto(mlir::DialectRegistry &registry) {
            registeriQoalaTranslation<mlir::arith::ArithDialect, ArithToiQoalaTranslation>(registry);
        }
    };
} // namespace qoala::translate

#endif // ARITHTOIQOALATRANSLATION_H
