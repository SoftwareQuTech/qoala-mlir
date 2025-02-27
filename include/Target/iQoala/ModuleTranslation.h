#ifndef MODULETRANSLATION_H
#define MODULETRANSLATION_H

#include "mlir/IR/Operation.h"
#include "mlir/IR/BuiltinOps.h"
#include "llvm/ADT/StringRef.h"
#include "Target/iQoala/Export.h"
#include "Target/iQoala/QoalaTranslationInterface.h"

namespace qoala::translate {
    class ModuleTranslation {
        friend std::unique_ptr<iqoala::iQoalaModule>
        translateModuleToiQoala(mlir::Operation *originalModule, iqoala::iQoalaContext &iQoalaContext,
                                llvm::StringRef name);
    private:
        ModuleTranslation(mlir::ModuleOp *module,
                          std::unique_ptr<iqoala::iQoalaModule> &iQoalaModule);
	mlir::ModuleOp *mlirModule;
        std::unique_ptr<iqoala::iQoalaModule> iQoalaModule;
        QoalaTranslationInterfaces iface;
        // TODO - Define the public functions that we need to place in this class
    public:
	mlir::LogicalResult convertOperation(mlir::Operation &op);
	mlir::LogicalResult convertFunctionSignatures() const;
        void addRemoteDeclaration(llvm::StringRef remoteName) const;
        void setModuleName(llvm::StringRef moduleName) const;

        [[nodiscard]]
        mlir::ModuleOp *getModule() const { return mlirModule; }
    };
}

#endif //MODULETRANSLATION_H
