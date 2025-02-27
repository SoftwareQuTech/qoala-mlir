#ifndef QOALATRANSLATIONS_H
#define QOALATRANSLATIONS_H

#include "mlir/IR/DialectRegistry.h"

#include "Target/iQoala/Dialect/QRemote/QRemoteToiQoalaTranslation.h"
#include "Target/iQoala/Dialect/NetQASM/NetQASMToiQoalaTranslation.h"
#include "Target/iQoala/Dialect/QoalaHost/QoalaHostToiQoalaTranslation.h"
#include "Target/iQoala/Dialect/Arith/ArithToiQoalaTranslation.h"
#include "Target/iQoala/Dialect/Builtin/BuiltinToiQoalaTranslation.h"
#include "Target/iQoala/Dialect/Tensor/TensorToiQoalaTranslation.h"

namespace qoala::translate {
    inline void registerAllQoalaTranslations(mlir::DialectRegistry &registry) {
        // Simply insert the qoala dialects and the translation interfaces for exporting Qoala LIR
        QRemoteToiQoalaTranslation::registerInto(registry);
        NetQASMToiQoalaTranslation::registerInto(registry);
        QoalaHostToiQoalaTranslation::registerInto(registry);
    }

    inline void registerAllQoalaSupportTranslations(mlir::DialectRegistry &registry) {
        // We insert the translation of other "helper" dialects
        ArithToiQoalaTranslation::registerInto(registry);
        BuiltinToiQoalaTranslation::registerInto(registry);
        TensorToiQoalaTranslation::registerInto(registry);
    }
}

#endif //QOALATRANSLATIONS_H
