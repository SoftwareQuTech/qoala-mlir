#ifndef QOALA_MLIR_TENSORTOIQOALATRANSLATION_H
#define QOALA_MLIR_TENSORTOIQOALATRANSLATION_H

namespace mlir {
    class DialectRegistry;
}

namespace qoala::translate {
    void registerTensorToiQoalaTranslations(mlir::DialectRegistry &registry);
}

#endif //QOALA_MLIR_TENSORTOIQOALATRANSLATION_H
