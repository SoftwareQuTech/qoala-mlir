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
    if (!opIface) {
        return op.emitError("cannot be converted to iQoala: missing "
                            "`QoalaTranslationDialectInterface` registration for "
                            "dialect for op: ")
                << op.getName();
    }
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
    // First, we translate the module itself
    if (failed(moduleTranslation.convertOperation(*originalModule))){
        return nullptr;
    }

    // Then we explore all the operations in the body
    for (Operation &op : getModuleBody(*originalModule).getOperations()) {
        if (failed(moduleTranslation.convertOperation(op))) {
            return nullptr;
        }
    }
    return nullptr;
}