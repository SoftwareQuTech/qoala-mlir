#ifndef ARITHTOIQOALATRANSLATION_H
#define ARITHTOIQOALATRANSLATION_H

namespace mlir {
    class DialectRegistry;
}

namespace qoala::translate {
    void registerArithToiQoalaTranslations(mlir::DialectRegistry &registry);
}

#endif //ARITHTOIQOALATRANSLATION_H
