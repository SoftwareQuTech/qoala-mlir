#ifndef MODULETRANSLATION_H
#define MODULETRANSLATION_H

#include "mlir/IR/Operation.h"
#include "mlir/IR/BuiltinOps.h"
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

	    mlir::LogicalResult convertOperation(mlir::Operation &op);
	    mlir::LogicalResult convertFunctionSignatures();
        void addRemoteDeclaration(llvm::StringRef remoteName) const;
        void setModuleName(llvm::StringRef moduleName) const;

        /* Block-related functions */
        void emplaceNewBlockInHostSection(mlir::Block *mlirBlock);
        iqoala::Block *getMappediQoalaBlock(const mlir::Block *mlirBlock) const;

        /* Mapping functions */
        void mapValue(const mlir::Value &mlirVal, assembly::iQoalaRegReference *regRef);
        [[nodiscard]]
        assembly::iQoalaRegReference *getMappedRegReference(const mlir::Value &mlirVal) const;
        void mapCmpValue(const mlir::Value &mlirVal, mlir::Operation *mlirOp);
        [[nodiscard]]
        mlir::Operation *getMappedCmpOperation(const mlir::Value &mlirVal) const;
        mlir::LogicalResult loadClassicalArgWithCallConv(iqoala::LocalQuantumRoutine *routine,
            mlir::Operation *localRoutineOp, const mlir::Value &mlirArgValue, uint8_t paramNum);

        [[nodiscard]]
        mlir::ModuleOp *getMLIRModule() const;
        [[nodiscard]]
        iqoala::iQoalaModule *getQoalaModule() const;
    private:
        [[nodiscard]]
        assembly::iQoalaRegReference *getMappedLocalRegReference(const mlir::Value &mlirVal) const;
        [[nodiscard]]
        assembly::iQoalaRegReference *getMappedQuantumRegReference(const mlir::Value &mlirVal) const;

        mlir::ModuleOp *mlirModule;
        std::unique_ptr<iqoala::iQoalaModule> iQoalaModule;
        QoalaTranslationInterfaces ifaces;
        // Mappings MLIR and MC objects
        mlir::DenseMap<mlir::Block *, iqoala::Block *> qoalaHostBlocksMap;
        mlir::DenseMap<mlir::Value, assembly::iQoalaRegReference *> localRegsMap;
        mlir::DenseMap<mlir::Value, assembly::iQoalaRegReference *> quantumRegsMap;
        // Map for comparison instructions
        // When encountering a cf.cond_br instruction, we should look into this map
        // to check for the corresponding comparison operation
        // The static type of the key is mlir::Operation *, but they are safe to dyn_cast
        // (using MLIR's dyn_cast) to arith::CmpIOp.
        mlir::DenseMap<mlir::Value, mlir::Operation *> cmpMap;
    };
}

#endif //MODULETRANSLATION_H
