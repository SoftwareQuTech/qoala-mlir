#ifndef QOALATRANSLATIONS_H
#define QOALATRANSLATIONS_H

#include "Target/iQoala/Dialect/QRemote/QRemoteToiQoalaTranslation.h"
#include "Target/iQoala/Dialect/NetQASM/NetQASMToiQoalaTranslation.h"
#include "Target/iQoala/Dialect/QoalaHost/QoalaHostToiQoalaTranslation.h"

namespace mlir {
    class DialectRegistry;
}

using namespace mlir;

namespace qoala::translate {
    inline void registerAllQoalaTranslations(DialectRegistry &registry) {
        registerQMemToiQoalaTranslations(registry);
        registerNetQASMToiQoalaTranslations(registry);
        registerQoalaHostToiQoalaTranslations(registry);
    }
    inline void registerAllQoalaSupportTranslations(DialectRegistry &registry) {
        // TODO - Add translation registrations here
    }
}

#endif //QOALATRANSLATIONS_H
