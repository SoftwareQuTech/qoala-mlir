#ifndef MODULETRANSLATION_H
#define MODULETRANSLATION_H

#include <stack>

#include "Target/iQoala/Export.h"
#include "Target/iQoala/QoalaTranslationInterface.h"
#include "Target/iQoala/iQoala.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Operation.h"
#include "mlir/Support/LLVM.h"

namespace qoala::translate {
    class ModuleTranslation {
        /**
         * This class represents a frame on the emulated execution stack. It simply serves
         * as a "unique identifier" wrapper for the call operation. This is necessary since
         * despite we can push multiple times the same value (Operation *) into the stack, we
         * cannot place the same value (Operation *) into a map. The latter is needed to map
         * the MLIR values to the respective register references when emulating the execution
         * order and allocate the registers.
         */
        class ModuleStackFrame {
        public:
            ModuleStackFrame() = delete;

            explicit ModuleStackFrame(mlir::Operation *op): operation(op) { }

            /* Methods for mapping values */
            void mapValueInScope(const mlir::Value &value, assembly::iQoalaRegReference *regRef);
            [[nodiscard]]
            bool isValueInScope(const mlir::Value &value) const;
            [[nodiscard]]
            assembly::iQoalaRegReference *getRegReferenceForValue(const mlir::Value &value) const;
            void mapQubitIDInScope(const mlir::Value &value, uint8_t qubitID);
            void unmapQubitInScope(uint8_t qubitID);
            [[nodiscard]]
            uint8_t getMappedQubitInScope(const mlir::Value &value) const;
            [[nodiscard]]
            bool qubitValueWasReleased(const mlir::Value &value) const;

            [[nodiscard]]
            mlir::Operation *getOperation() const {
                return this->operation;
            }

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
            mlir::DenseMap<mlir::Value, assembly::iQoalaRegReference *> valuesInScope;
            mlir::DenseMap<mlir::Value, uint8_t> valuesToQubitIDs;
            mlir::DenseSet<mlir::Value> freedQubitValues;
        };

        /**
         * This class represents the "stack" when translating the module
         * It contains reference of the translated frames (starting from the MainFuncOp)
         * but also it keeps a map of the mapped MLIR values *in their respective scopes*.
         * This allows mapping the same value multiple times, since mapping a value will
         * be bound to the current scope on the stack.
         */
        class ModuleStack {
        public:
            ModuleStack() = default;
            ~ModuleStack();
            /* Stack handling methods */
            void pushNewStackFrame(mlir::Operation *op);
            [[nodiscard]]
            ModuleStackFrame *peekFrame();
            void popFrame();

            /* Methods for mapping values */
            void mapValueInCurrentStackFrame(const mlir::Value &value, assembly::iQoalaRegReference *regRef);
            bool valueIsMapped(const mlir::Value &value);

            /* Specific methods for mapping MLIR values to qubit IDs. We need these MLIR values *will not*
             * be mapped to a iQoalaRegReference */
            void mapValueToQubitID(const mlir::Value &value, uint8_t qubitID);
            void unmapQubitID(uint8_t qubitID);
            [[nodiscard]]
            bool valueIsMappedToQubitIDInCurrentStackFrame(const mlir::Value &value) const;
            [[nodiscard]]
            uint8_t getMappedQubitIDInCurrentStackFrame(const mlir::Value &value) const;
            [[nodiscard]]
            bool qubitValueWasFreedInCurrentStackFrame(const mlir::Value &value) const;

            assembly::iQoalaRegReference *getRegRefForValue(const mlir::Value &value);

        private:
            std::stack<ModuleStackFrame *> frames;
        };

    public:
        friend std::unique_ptr<iqoala::iQoalaModule> translateModuleToiQoala(mlir::Operation *originalModule,
                                                                             iqoala::iQoalaContext &iQoalaContext,
                                                                             llvm::StringRef name);
        ModuleTranslation(mlir::ModuleOp *module, std::unique_ptr<iqoala::iQoalaModule> &iQoalaModule);

        mlir::LogicalResult convertOperation(mlir::Operation &op);
        mlir::LogicalResult convertFunctionSignatures();
        [[nodiscard]]
        bool addRemoteDeclaration(llvm::StringRef remoteName, bool classicalSocket, bool eprsSocket) const;
        [[nodiscard]]
        std::optional<uint8_t> getEPRSocketIDForRemote(llvm::StringRef remoteName) const;
        [[nodiscard]]
        std::optional<uint8_t> getClassicalSocketIDForRemote(llvm::StringRef remoteName) const;
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
        assembly::iQoalaRegReference *getMappedRegRefForValue(const mlir::Value &mlirVal, bool copy = true);
        bool valueIsMappedInCurrentFrame(const mlir::Value &value);
        void mapValueToQubitID(const mlir::Value &value, uint8_t qubitID);
        void unmapQubitID(uint8_t qubitID);
        [[nodiscard]]
        uint8_t getMappedQubitID(const mlir::Value &value) const;
        [[nodiscard]]
        bool valueIsMappedToQubit(const mlir::Value &value) const;
        [[nodiscard]]
        bool qubitValueWasReleased(const mlir::Value &value) const;
        void mapCmpValue(const mlir::Value &mlirVal, mlir::Operation *mlirOp);
        [[nodiscard]]
        mlir::Operation *getMappedCmpOperation(const mlir::Value &mlirVal) const;
        mlir::BlockArgument getValueForMCInstruction(const assembly::iQoalaMCInstruction *mcInst) const;

        [[nodiscard]]
        mlir::ModuleOp *getMLIRModule() const;
        [[nodiscard]]
        iqoala::iQoalaModule *getQoalaModule() const;

        [[nodiscard]]
        std::optional<iqoala::Block *> findIdPrecedence(const llvm::StringRef &key) const;

        void addIdPrecedence(const llvm::StringRef &key, iqoala::Block *block) {
            this->precedencesIdsToIQoalaBlocks[key] = block;
        }

        [[nodiscard]]
        assembly::iQoalaRegReference *getRegRefForCSocketName(mlir::StringRef remoteName) const;
        void setRegRefForCSocketName(const llvm::StringRef &remoteName, assembly::iQoalaRegReference *regRef);

    protected:
        /* Functions for following "call convention" for arguments in the local quantum routines */
        mlir::LogicalResult loadClassicalArgWithCallConv(const mlir::BlockArgument &blockArg,
                                                         iqoala::LocalQuantumRoutine *iQoalaRoutine,
                                                         mlir::Operation *localRoutineOp, uint32_t argIndex);
        mlir::LogicalResult loadQuantumArgWithCallConv(const mlir::BlockArgument &blockArg,
                                                      iqoala::QuantumRoutine *iQoalaRoutine,
                                                      mlir::Operation *localRoutineOp);
        mlir::LogicalResult convertLocalRoutines();
        mlir::LogicalResult convertRequestRoutines() const;

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
        // Map for tracking MLIR values of function arguments to the MC instruction
        // that are used to retrieve those values in the MC model
        mlir::DenseMap<assembly::iQoalaMCInstruction *, mlir::BlockArgument> mcArgsValsMap;

        // Map for tracking the precedence ID (added by the AddBlockPrecedences pass) and its Block
        llvm::StringMap<iqoala::Block *> precedencesIdsToIQoalaBlocks;
        // Map for tracking the classical remote id (csocket) references to the iQoalaRegRefs that hold those values.
        mlir::DenseMap<llvm::StringRef, assembly::iQoalaRegReference *> csocketsMap;
    };
} // namespace qoala::translate

#endif // MODULETRANSLATION_H
