#ifndef MODULETRANSLATION_H
#define MODULETRANSLATION_H

#include <stack>

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

        /* Stack manipulation while analyzing the module */
        void pushFrame(mlir::Operation *op);
        mlir::Operation *peekFrame();
        mlir::Operation *popFrame();

        /* Mapping functions */
        void mapValueForRoutine(const mlir::Value &mlirVal, const std::optional<mlir::Operation *> &routine,
            assembly::iQoalaRegReference *regRef);
        // TODO - remove the default value of the second argument!
        [[nodiscard]]
        assembly::iQoalaRegReference *getMappedRegRefForRoutine(const mlir::Value &mlirVal,
            const std::optional<mlir::Operation *> &routine = std::nullopt) const;
        void mapCmpValue(const mlir::Value &mlirVal, mlir::Operation *mlirOp);
        [[nodiscard]]
        mlir::Operation *getMappedCmpOperation(const mlir::Value &mlirVal) const;

        /* Functions for following "call convention" for arguments in the local quantum routines */
        mlir::LogicalResult loadClassicalArgWithCallConv(iqoala::LocalQuantumRoutine *iQoalaRoutine,
            mlir::Operation *localRoutineOp, const mlir::Value &localRoutineArgVal, uint8_t argIndex);
        mlir::LogicalResult loadQuantumArgWithCalConv(iqoala::QuantumRoutine *iQoalaRoutine,
            mlir::Operation *localRoutineOp, const mlir::Value &qoalaHostQubitVal,
            const mlir::Value &localRoutineArgVal, uint8_t argIndex);

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
        // Quantum registers map are a bit more complex.
        // Since quantum routines can be called more than once, we need to allow mapping the
        // value to multiple register references, depending on the call operation.
        mlir::DenseMap<mlir::Value, assembly::iQoalaRegReference *> quantumRegsMap;
        std::stack<mlir::Operation *> translationStack;
        // Map for comparison instructions
        // When encountering a cf.cond_br instruction, we should look into this map
        // to check for the corresponding comparison operation
        // The static type of the key is mlir::Operation *, but they are safe to dyn_cast
        // (using MLIR's dyn_cast) to arith::CmpIOp.
        mlir::DenseMap<mlir::Value, mlir::Operation *> cmpMap;
    };
}

#endif //MODULETRANSLATION_H
