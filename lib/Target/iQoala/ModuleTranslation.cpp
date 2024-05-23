#include "Target/iQoala/ModuleTranslation.h"
#include "Target/iQoala/QoalaTranslationInterface.h"
#include "mlir/IR/BuiltinOps.h"

using namespace qoala;
using namespace qoala::iqoala;

ModuleTranslation::ModuleTranslation (Operation *module,
                                      std::unique_ptr<iqoala::Module> iQoalaModule)
                                      : mlirModule(module), iQoalaModule(std::move(iQoalaModule)),
                                      iface(module->getContext()) {/* TODO */}

LogicalResult ModuleTranslation::convertOperation(Operation &op) {
    const QoalaTranslationDialectInterface *opIface = iface.getInterfaceFor(&op);
    return opIface->convertOperation(&op, *this);
}


static inline Block &getModuleBody(Operation &module) {
    assert(llvm::isa<ModuleOp>(module));
    return module.getRegion(0).front();
}


std::unique_ptr<iqoala::Module> qoala::translate::translateModuleToiQoala(
        Operation *originalModule, iQoalaContext &iQoalaContext, llvm::StringRef name) {
    // TODO - Entry point for the transformations
    ModuleTranslation moduleTranslation(originalModule, nullptr);
    for (Operation &op : getModuleBody(*originalModule).getOperations()) {
        if (failed(moduleTranslation.convertOperation(op))) {
            return nullptr;
        }
    }
    return nullptr;
}