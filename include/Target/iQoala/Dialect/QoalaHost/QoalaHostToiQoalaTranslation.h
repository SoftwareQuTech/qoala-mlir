#ifndef QOALAHOSTTOIQOALATRANSLATION_H
#define QOALAHOSTTOIQOALATRANSLATION_H

namespace mlir {
    class DialectRegistry;
}

namespace qoala::translate {
    void registerQoalaHostToiQoalaTranslations(mlir::DialectRegistry &registry);
}

#endif //QOALAHOSTTOIQOALATRANSLATION_H
