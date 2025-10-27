#ifndef QOALA_MLIR_EXPORT_H
#define QOALA_MLIR_EXPORT_H

#include <memory>
#include "Target/iQoala/Module.h"
#include "Target/iQoala/iQoalaContext.h"
#include "llvm/ADT/StringRef.h"
#include "mlir/IR/Operation.h"

namespace qoala::translate {
    std::unique_ptr<iqoala::iQoalaModule> translateModuleToiQoala(mlir::Operation *originalModule,
                                                                  iqoala::iQoalaContext &iQoalaContext,
                                                                  llvm::StringRef name = "iQoalaModule");
}

#endif // QOALA_MLIR_EXPORT_H
