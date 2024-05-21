#ifndef QOALA_MLIR_EXPORT_H
#define QOALA_MLIR_EXPORT_H

#include "llvm/ADT/StringRef.h"
#include <memory>

namespace qoala::iqoala {
    // TODO - Fully implement these classes

    class iQoalaContext {
    public:
        iQoalaContext() {}
        ~iQoalaContext() {}
    };
    class Module {
    public:
        Module() {}
        ~Module() {}
    };
} // namespace qoala::iqoala

namespace mlir {
    class Operation;
}

namespace qoala::translate {
    std::unique_ptr<qoala::iqoala::Module>
    translateModuleToiQoala(mlir::Operation *module, qoala::iqoala::iQoalaContext &iQoalaContext,
                            llvm::StringRef name = "iQoalaModule");
}

#endif //QOALA_MLIR_EXPORT_H
