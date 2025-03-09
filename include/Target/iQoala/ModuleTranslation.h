#ifndef MODULETRANSLATION_H
#define MODULETRANSLATION_H

#include "mlir/IR/Operation.h"
#include "mlir/IR/BuiltinOps.h"
#include "llvm/ADT/StringRef.h"
#include "mlir/Support/LLVM.h"
#include "Target/iQoala/Export.h"
#include "Target/iQoala/iQoala.h"
#include "Target/iQoala/QoalaTranslationInterface.h"

namespace qoala::translate {
    class ModuleTranslation {
    public:
        friend std::unique_ptr<iqoala::iQoalaModule>
        translateModuleToiQoala(mlir::Operation *originalModule, iqoala::iQoalaContext &iQoalaContext,
                                llvm::StringRef name);
        ModuleTranslation(mlir::ModuleOp *module,
                          std::unique_ptr<iqoala::iQoalaModule> &iQoalaModule);

        // TODO - Define the public functions that we need to place in this class
	    mlir::LogicalResult convertOperation(mlir::Operation &op);
	    mlir::LogicalResult convertFunctionSignatures() const;
        void addRemoteDeclaration(llvm::StringRef remoteName) const;
        void setModuleName(llvm::StringRef moduleName) const;

        /* Block-related functions */
        iqoala::Block *emplaceNewBlockInHostSection(mlir::Block *mlirBlock);
        iqoala::Block *getMappediQoalaBlock(const mlir::Block *mlirBlock) const;

        /* Mapping functions */
        void mapValue(const mlir::Value &mlirVal, assembly::iQoalaRegReference *regRef);

        [[nodiscard]]
        mlir::ModuleOp *getMLIRModule() const;
        [[nodiscard]]
        iqoala::iQoalaModule *getQoalaModule() const;
    private:
        mlir::ModuleOp *mlirModule;
        std::unique_ptr<iqoala::iQoalaModule> iQoalaModule;
        QoalaTranslationInterfaces iface;
        // Mappings MLIR and MC objects
        mlir::DenseMap<mlir::Block *, iqoala::Block *> blocksMap;
        mlir::DenseMap<mlir::Value, assembly::iQoalaRegReference *> localRegsMap;
        mlir::DenseMap<mlir::Value, assembly::iQoalaRegReference *> quantumRegsMap;
    };
}

#endif //MODULETRANSLATION_H
