#include "Target/iQoala/ModuleTranslation.h"
#include "Target/iQoala/QoalaTranslationInterface.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "mlir/IR/BuiltinOps.h"
#include "llvm/Support/Debug.h"

using namespace mlir;
using namespace qoala;
using namespace qoala::translate;
using namespace qoala::iqoala;
using namespace qoala::dialects::netqasm;

#define DEBUG_TYPE "module-translate"

// A helper function to obtain the body of given module
static inline mlir::Block &getModuleBody(Operation &module) {
    assert(llvm::isa<ModuleOp>(module));
    return module.getRegion(0).front();
}

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

void ModuleTranslation::addRemoteDeclaration(const StringRef remoteName) const {
    this->iQoalaModule->addRemoteDeclaration(remoteName);
}

void ModuleTranslation::setModuleName(const StringRef moduleName) const {
    this->iQoalaModule->setModuleName(moduleName);
}

LogicalResult ModuleTranslation::convertFunctionSignatures() const {
    for (auto localRoutine : getModuleBody(*mlirModule->getOperation()).getOps<LocalRoutineOp>()) {
        // TODO - Implement how to handle the conversion of local routines
        if (localRoutine.getName() == "__qoala_convert_float_angle") {
            // TODO - "__qoala_convert_float_angle" is a "routine" of this type: handle it specifically
        } else {
            LocalQuantumRoutine routine(localRoutine.getName());
            iQoalaModule->addRoutine(routine);
        }
    }
    for (auto localRoutine : getModuleBody(*mlirModule->getOperation()).getOps<RequestRoutineOp>()) {
        // TODO - Implement how to handle the conversion of request routines
    }
    return success();
}


std::unique_ptr<iQoalaModule> translate::translateModuleToiQoala(
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

    // Second, we translate the function signatures
    if (failed(moduleTranslation.convertFunctionSignatures())) {
        return nullptr;
    }

    // TODO - We might need to explore other module-level operations separately

    // Then we explore all the operations in the body
    for (Operation &op : getModuleBody(*originalModule).getOperations()) {
        if (failed(moduleTranslation.convertOperation(op))) {
            return nullptr;
        }
    }
    return std::move(moduleTranslation.iQoalaModule);
}