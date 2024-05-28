#ifndef BUILTINTOIQOALATRANSLATION_H
#define BUILTINTOIQOALATRANSLATION_H

namespace mlir {
    class DialectRegistry;
}

namespace qoala::translate {
    void registerBuiltinToiQoalaTranslations(mlir::DialectRegistry &registry);
}


#endif //BUILTINTOIQOALATRANSLATION_H
