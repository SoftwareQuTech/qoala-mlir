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
        registerQRemoteToiQoalaTranslations(registry);
        registerNetQASMToiQoalaTranslations(registry);
        registerQoalaHostToiQoalaTranslations(registry);
    }

    inline void registerAllQoalaSupportTranslations(mlir::DialectRegistry &registry) {
        // We insert the translation of other "helper" dialects
        // TODO - Add translation registrations here
        registerArithToiQoalaTranslations(registry);
        registerBuiltinToiQoalaTranslations(registry);
        registerTensorToiQoalaTranslations(registry);
    }
}

#endif //QOALATRANSLATIONS_H
