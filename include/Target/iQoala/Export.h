#ifndef QOALA_MLIR_EXPORT_H
#define QOALA_MLIR_EXPORT_H

#include "llvm/ADT/StringRef.h"
#include "mlir/IR/Operation.h"
#include "Target/iQoala/Module.h"
#include "Target/iQoala/iQoalaContext.h"
#include <memory>

namespace qoala::translate {
    std::unique_ptr<iqoala::iQoalaModule>
    translateModuleToiQoala(mlir::Operation *module, iqoala::iQoalaContext &iQoalaContext,
                            llvm::StringRef name = "iQoalaModule");
}

#endif //QOALA_MLIR_EXPORT_H
