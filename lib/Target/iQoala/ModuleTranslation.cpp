#include "Target/iQoala/ModuleTranslation.h"
#include "Target/iQoala/QoalaTranslationInterface.h"
#include "mlir/IR/BuiltinOps.h"
#include "llvm/Support/Debug.h"

using namespace qoala;
using namespace qoala::iqoala;

#define DEBUG_TYPE "module-translate"

ModuleTranslation::ModuleTranslation (ModuleOp *module,
                                      std::unique_ptr<iqoala::iQoalaModule> &iQoalaModule)
                                      : mlirModule(module), iQoalaModule(std::move(iQoalaModule)),
                                      iface(module->getContext()) { }

LogicalResult ModuleTranslation::convertOperation(Operation &op) {
    // This is the entry point of the translation of any operation.
    // It simply tries to get a registered translation class for the operation (type)
    // and invokes the "convertOperation" method on it.
    const QoalaTranslationDialectInterface *opIface = iface.getInterfaceFor(&op);
    if (!opIface) {
        return op.emitError("cannot be converted to iQoala: missing "
                            "`QoalaTranslationDialectInterface` registration for "
                            "dialect for op: ")
                << op.getName();
    }
    return opIface->convertOperation(&op, *this);
}

void ModuleTranslation::addRemoteDeclaration(const llvm::StringRef remoteName) const {
    this->iQoalaModule->addRemoteDeclaration(remoteName);
}

void ModuleTranslation::setModuleName(const llvm::StringRef moduleName) const {
    this->iQoalaModule->setModuleName(moduleName);
}


static inline mlir::Block &getModuleBody(Operation &module) {
    assert(llvm::isa<ModuleOp>(module));
    return module.getRegion(0).front();
}


std::unique_ptr<iqoala::iQoalaModule> qoala::translate::translateModuleToiQoala(
        Operation *originalModule, iQoalaContext &iQoalaContext, llvm::StringRef name) {
    // Entry point for the transformations
    auto iQoalaModule = std::make_unique<iqoala::iQoalaModule>(name, iQoalaContext);
    auto mlirModule = dyn_cast<ModuleOp>(originalModule);
    ModuleTranslation moduleTranslation(&mlirModule, iQoalaModule);
    // First, we translate the module itself
    LLVM_DEBUG(llvm::dbgs() << "******** Translating module '" << originalModule->getName() << "' *********\n");
    LLVM_DEBUG(originalModule->dump());
    if (failed(moduleTranslation.convertOperation(*originalModule))){
        return nullptr;
    }

    // Then we explore all the operations in the body
    for (Operation &op : getModuleBody(*originalModule).getOperations()) {
        if (failed(moduleTranslation.convertOperation(op))) {
            return nullptr;
        }
    }
    return std::move(moduleTranslation.iQoalaModule);
}