#ifndef MODULETRANSLATION_H
#define MODULETRANSLATION_H

#include "Target/iQoala/Export.h"
#include "Target/iQoala/QoalaTranslationInterface.h"
#include "mlir/IR/Operation.h"

using namespace mlir;
using namespace qoala;

namespace qoala::translate {
    class ModuleTranslation {
        friend std::unique_ptr<iqoala::iQoalaModule>
        qoala::translate::translateModuleToiQoala(Operation *originalModule, iqoala::iQoalaContext &iQoalaContext,
                                                  llvm::StringRef name);
    private:
        ModuleTranslation(ModuleOp *module,
                          std::unique_ptr<iqoala::iQoalaModule> &iQoalaModule);
        ModuleOp *mlirModule;
        std::unique_ptr<iqoala::iQoalaModule> iQoalaModule;
        iqoala::QoalaTranslationInterfaces iface;
        // TODO - Define the public functions that we need to place in this class
    public:
        LogicalResult convertOperation(Operation &op);
        LogicalResult convertFunctionSignatures() const;
        void addRemoteDeclaration(StringRef remoteName) const;
        void setModuleName(StringRef moduleName) const;

        [[nodiscard]]
        ModuleOp *getModule() const { return mlirModule; }
    };
}

#endif //MODULETRANSLATION_H
