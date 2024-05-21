#ifndef QOALATRANSLATIONS_H
#define QOALATRANSLATIONS_H

#include "Target/iQoala/Dialect/QRemote/QRemoteToiQoalaTranslation.h"
#include "Target/iQoala/Dialect/NetQASM/NetQASMToiQoalaTranslation.h"
#include "Target/iQoala/Dialect/QoalaHost/QoalaHostToiQoalaTranslation.h"
#include "Target/iQoala/Dialect/Arith/ArithToiQoalaTranslation.h"
#include "Target/iQoala/Dialect/Tensor/TensorToiQoalaTranslation.h"

namespace mlir {
    class DialectRegistry;
}

using namespace mlir;

namespace qoala::translate {
    inline void registerAllQoalaTranslations(DialectRegistry &registry) {
        // Simply insert the qoala dialects and the translation interfaces for exporting Qoala LIR
        registerQMemToiQoalaTranslations(registry);
        registerNetQASMToiQoalaTranslations(registry);
        registerQoalaHostToiQoalaTranslations(registry);
    }

    inline void registerAllQoalaSupportTranslations(DialectRegistry &registry) {
        // We insert the translation of other "helper" dialects
        // TODO - Add translation registrations here
        registerArithToiQoalaTranslations(registry);
        registerTensorToiQoalaTranslations(registry);
    }
}

#endif //QOALATRANSLATIONS_H
