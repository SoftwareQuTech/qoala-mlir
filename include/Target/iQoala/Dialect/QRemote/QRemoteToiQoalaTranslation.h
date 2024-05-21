#ifndef QREMOTETOIQOALATRANSLATION_H
#define QREMOTETOIQOALATRANSLATION_H

namespace mlir {
    class DialectRegistry;
}

namespace qoala::translate {
    void registerQMemToiQoalaTranslations(mlir::DialectRegistry &registry);
}

#endif //QREMOTETOIQOALATRANSLATION_H
