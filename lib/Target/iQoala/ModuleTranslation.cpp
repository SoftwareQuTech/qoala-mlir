#include "Target/iQoala/ModuleTranslation.h"
#include "mlir/IR/BuiltinOps.h"

using namespace qoala;

ModuleTranslation::ModuleTranslation (Operation *module,
                                      std::unique_ptr<iqoala::Module> iQoalaModule)
                                      : mlirModule(module), iQoalaModule(std::move(iQoalaModule)),
                                      iface(module->getContext()) {
    // TODO
}

LogicalResult ModuleTranslation::convertOperation(mlir::Operation &op, bool recordInsertions) {
    return success();
}


static inline Block &getModuleBody(Operation &module) {
    assert(llvm::isa<ModuleOp>(module));
    return module.getRegion(0).front();
}


std::unique_ptr<iqoala::Module> qoala::translate::translateModuleToiQoala(
        mlir::Operation *originalModule, qoala::iqoala::iQoalaContext &iQoalaContext,
        llvm::StringRef name) {
    // TODO - Entry point for the transformations
    ModuleTranslation moduleTranslation(originalModule, nullptr);
    for (Operation &op : getModuleBody(*originalModule).getOperations()) {
    }
    return nullptr;
}