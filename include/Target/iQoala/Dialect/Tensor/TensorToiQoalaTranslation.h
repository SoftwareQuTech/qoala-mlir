#ifndef QOALA_MLIR_TENSORTOIQOALATRANSLATION_H
#define QOALA_MLIR_TENSORTOIQOALATRANSLATION_H

#include "Target/iQoala/ModuleTranslation.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/IR/Operation.h"

#include "mlir/Dialect/Tensor/IR/Tensor.h"

namespace qoala::translate {
    class TensorToiQoalaTranslation : public QoalaTranslationDialectInterface {
    public:
        using QoalaTranslationDialectInterface::QoalaTranslationDialectInterface;

        mlir::LogicalResult convertOperation(mlir::Operation *op, ModuleTranslation *moduleTranslation) const final;

        static void registerInto(mlir::DialectRegistry &registry) {
            registeriQoalaTranslation<mlir::tensor::TensorDialect, TensorToiQoalaTranslation>(registry);
        }
    };
}

#endif //QOALA_MLIR_TENSORTOIQOALATRANSLATION_H
