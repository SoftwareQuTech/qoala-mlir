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
        /**
         * This class represents a frame on the emulated execution stack. It simply serves
         * as a "unique identifier" wrapper for the call operation. This is necessary since
         * despite we can push multiple times the same value (Operation *) into the stack, we
         * cannot place the same value (Operation *) into a map, which is needed to map
         * the MLIR values to the respective register references when emulating the execution
         * order and allocate the registers.
         */
        class ModuleStackFrame {
        public:
            ModuleStackFrame() = delete;
            explicit ModuleStackFrame(mlir::Operation *op) : operation(op) {}

            /* Methods for mapping values */
            void mapValueInScope(const mlir::Value &value, assembly::iQoalaRegReference *regRef);
            [[nodiscard]]
            bool isValueInScope(const mlir::Value &value) const;
            [[nodiscard]]
            assembly::iQoalaRegReference *getRegReferenceForValue(const mlir::Value &value) const;

            [[nodiscard]]
            mlir::Operation *getOperation() const { return this->operation; }
            [[nodiscard]]
            bool isModule() const;
            [[nodiscard]]
            bool isQoalaHost() const;
            [[nodiscard]]
            bool isLocalRoutine() const;
            [[nodiscard]]
            bool isRequestRoutine() const;
        private:
            mlir::Operation *operation;
            mlir::DenseMap<mlir::Value, assembly::iQoalaRegReference *>valuesInScope;
        };
        /**
         * This class represents the "stack" when translating the module
         * It contains reference of the translated frames (starting from the MainFuncOp)
         * but also it keeps a map of the mapped MLIR values *in their respective scopes*.
         * This allows mapping the same value multiple times, since it will be
         */
        class ModuleStack {
        public:
            /* Stack handling methods */
            void pushNewStackFrame(mlir::Operation *op);
            [[nodiscard]]
            ModuleStackFrame *peekFrame();
            void popFrame();

            /* Methods for mapping values */
            void mapValueInCurrentStackFrame(const mlir::Value &value, assembly::iQoalaRegReference *regRef);
            bool valueIsMapped(const mlir::Value &value);
            assembly::iQoalaRegReference *getRegRefForValue(const mlir::Value &value);

        private:
            std::stack<ModuleStackFrame *> frames;
        };
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
        void pushNewFrame(mlir::Operation *op);
        mlir::Operation *peekFrame();
        mlir::Operation *popFrame();

        /* Functions for Mapping MLIR values to the RegReference objects
         * These functions search for the respective value using the emulated stack
         */
        void mapValueToRegRef(const mlir::Value &mlirVal, assembly::iQoalaRegReference *regRef);
        [[nodiscard]]
        assembly::iQoalaRegReference *getMappedRegRefForValue(const mlir::Value &mlirVal);
        bool valueIsMappedInCurrentFrame(const mlir::Value &value);
        bool valueIsMappedToQubitInCurrentFrame(const mlir::Value &value);
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
        mlir::ModuleOp *mlirModule;
        std::unique_ptr<iqoala::iQoalaModule> iQoalaModule;
        QoalaTranslationInterfaces ifaces;
        // Mappings MLIR and MC objects
        mlir::DenseMap<mlir::Block *, iqoala::Block *> qoalaHostBlocksMap;
        ModuleStack translationStack;
        // Map for comparison instructions
        // When encountering a cf.cond_br instruction, we should look into this map
        // to check for the corresponding comparison operation
        // The static type of the key is mlir::Operation *, but they are safe to dyn_cast
        // (using MLIR's dyn_cast) to arith::CmpIOp.
        mlir::DenseMap<mlir::Value, mlir::Operation *> cmpMap;
    };
}

#endif //MODULETRANSLATION_H
