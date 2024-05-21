#ifndef QOALATRANSLATIONS_H
#define QOALATRANSLATIONS_H

namespace mlir {
    class DialectRegistry;
}

using namespace mlir;

namespace qoala::translate {
    inline void registerAllQoalaTranslations(DialectRegistry &regstry) {
        // TODO - Add translation registrations here
    }
    inline void registerAllQoalaSupportTranslations(DialectRegistry &registry) {
        // TODO - Add translation registrations here
    }
}

#endif //QOALATRANSLATIONS_H
