#ifndef QOALATRANSLATIONS_H
#define QOALATRANSLATIONS_H

#include "Target/iQoala/Dialect/QRemote/QRemoteToiQoalaTranslation.h"

namespace mlir {
    class DialectRegistry;
}

using namespace mlir;

namespace qoala::translate {
    inline void registerAllQoalaTranslations(DialectRegistry &regstry) {
        // TODO - Add translation registrations here
        registerQMemToiQoalaTranslations(regstry);
    }
    inline void registerAllQoalaSupportTranslations(DialectRegistry &registry) {
        // TODO - Add translation registrations here
    }
}

#endif //QOALATRANSLATIONS_H
