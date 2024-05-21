#include "Target/iQoala/ModuleTranslation.h"
#include "mlir/IR/BuiltinOps.h"

std::unique_ptr<qoala::iqoala::Module> qoala::translate::translateModuleToiQoala(
        mlir::Operation *originalModule, qoala::iqoala::iQoalaContext &iQoalaContext,
        llvm::StringRef name) {
    // TODO - Entry point for the transformations
    originalModule->dump();
    return nullptr;
}