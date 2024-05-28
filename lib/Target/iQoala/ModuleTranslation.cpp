#include "Target/iQoala/ModuleTranslation.h"
#include "Target/iQoala/QoalaTranslationInterface.h"
#include "mlir/IR/BuiltinOps.h"
#include "llvm/Support/Debug.h"

using namespace qoala;
using namespace qoala::iqoala;

#define DEBUG_TYPE "module-translate"

ModuleTranslation::ModuleTranslation (Operation *module,
                                      std::unique_ptr<iqoala::Module> &iQoalaModule)
                                      : mlirModule(module), iQoalaModule(std::move(iQoalaModule)),
                                      iface(module->getContext()) { }

LogicalResult ModuleTranslation::convertOperation(Operation &op) {
    const QoalaTranslationDialectInterface *opIface = iface.getInterfaceFor(&op);
    if (!opIface) {
        return op.emitError("cannot be converted to iQoala: missing "
                            "`QoalaTranslationDialectInterface` registration for "
                            "dialect for op: ")
                << op.getName();
    }
    return opIface->convertOperation(&op, *this);
}

void ModuleTranslation::addRemoteDeclaration(llvm::StringRef remoteName) {
    this->iQoalaModule->addRemoteDeclaration(remoteName);
}

void ModuleTranslation::setModuleName(llvm::StringRef moduleName) {
    this->iQoalaModule->setModuleName(moduleName);
}


static inline mlir::Block &getModuleBody(Operation &module) {
    assert(llvm::isa<ModuleOp>(module));
    return module.getRegion(0).front();
}


std::unique_ptr<iqoala::Module> qoala::translate::translateModuleToiQoala(
        Operation *originalModule, iQoalaContext &iQoalaContext, llvm::StringRef name) {
    // Entry point for the transformations
    auto iQoalaModule = std::make_unique<iqoala::Module>(name, iQoalaContext);
    ModuleTranslation moduleTranslation(originalModule, iQoalaModule);
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