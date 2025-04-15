#include "mlir/IR/BuiltinOps.h"

#include "Analysis/Helpers/Helpers.h"
#include "Analysis/NetQASM/Helpers.h"
#include "Conversion/Helpers/Helpers.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "Target/iQoala/Module.h"
#include "Target/iQoala/iQoala.h"
#include "Target/iQoala/MC/Helpers.h"
#include "Target/iQoala/ModuleTranslation.h"
#include "Target/iQoala/QoalaTranslationInterface.h"

#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "module-translate"

using namespace mlir;
using namespace qoala;
using namespace qoala::iqoala;
using namespace qoala::assembly;
using namespace qoala::analysis;
using namespace qoala::dialects::netqasm;
using namespace qoala::dialects::qoalahost;
using namespace qoala::dialects::qremote;

namespace qoala::translate {
#if  __cplusplus >= 202002L
    /* When using C++20 or newer standard, the format must be "python-style" */
    static const std::string paramNameFormat = "p{}";
#else
    /* When using C++17 or older standard, the format must be "C-style" */
    static const std::string paramNameFormat = "p%d";
#endif
    // A helper function to obtain the body of given module
    static mlir::Block &getModuleBody(Operation *module) {
        assert(isa<ModuleOp>(module));
        return module->getRegion(0).front();
    }

    ModuleOp *ModuleTranslation::getMLIRModule() const { return this->mlirModule; }
    iQoalaModule *ModuleTranslation::getQoalaModule() const { return this->iQoalaModule.get(); }

    ModuleTranslation::ModuleTranslation (ModuleOp *module,
                                          std::unique_ptr<iqoala::iQoalaModule> &iQoalaModule)
                                          : mlirModule(module), iQoalaModule(std::move(iQoalaModule)),
                                          ifaces(module->getContext()) { }

    LogicalResult ModuleTranslation::convertOperation(Operation &op) {
        // This is the entry point of the translation of any operation.
        // It simply tries to get a registered translation class for the operation (type)
        // and invokes the "convertOperation" method on it.
        const QoalaTranslationDialectInterface *opIface = this->ifaces.getInterfaceFor(&op);
        if (!opIface) {
            return op.emitOpError("cannot be converted to iQoala: missing "
                                  "`QoalaTranslationDialectInterface` registration for "
                                  "dialect for op: ");
        }
        return opIface->convertOperation(&op, this);
    }

    void ModuleTranslation::pushFrame(Operation *op) {
        assert((isa<MainFuncOp>(op) || isa<LocalRoutineOp>(op) || isa<RequestRoutineOp>(op)) &&
            "ModuleTranslation: trying to push an operation of an invalid type on the stack");
        this->translationStack.push(op);
    }

    Operation *ModuleTranslation::peekFrame() {
        return this->translationStack.top();
    }

    Operation *ModuleTranslation::popFrame() {
        Operation *op = this->peekFrame();
        this->translationStack.pop();
        return op;
    }

    void ModuleTranslation::addRemoteDeclaration(const StringRef remoteName) const {
        this->iQoalaModule->addRemoteDeclaration(remoteName);
    }

    void ModuleTranslation::setModuleName(const StringRef moduleName) const {
        this->iQoalaModule->setModuleName(moduleName);
    }

    void ModuleTranslation::emplaceNewBlockInHostSection(mlir::Block *mlirBlock) {
        auto *newBlock = this->iQoalaModule->addHostBlock();
        const auto result = this->qoalaHostBlocksMap.try_emplace(mlirBlock, newBlock);
        (void) result;
        assert(result.second && "Attempting to map a block that is already mapped");
    }

    void ModuleTranslation::mapValueForRoutine(const Value &mlirVal, const std::optional<Operation *> &routine,
        iQoalaRegReference *regRef) {
        if (regRef->isLocal()) {
            const auto result = this->localRegsMap.try_emplace(mlirVal, regRef);
            (void) result;
            assert(result.second && "Attempting to map a local value that is already mapped");
        }
        if (regRef->isQuantum()) {
            const auto result = this->quantumRegsMap.try_emplace(mlirVal, regRef);
            (void) result;
            LLVM_DEBUG(llvm::dbgs() << "***** mapping a quantum value " << mlirVal << "\n");
            assert(result.second && "Attempting to map a quantum value that is already mapped");
        }
    }

    iQoalaRegReference *ModuleTranslation::getMappedRegRefForRoutine(const Value &mlirVal,
        const std::optional<Operation *> &routine) const {
        // To ease the de-allocation process, we return copies of the references
        if (const auto localReg = this->getMappedLocalRegReference(mlirVal)) {
            return new iQoalaRegReference(*localReg);
        }
        if (const auto quantumReg = this->getMappedQuantumRegReference(mlirVal)) {
            return new iQoalaRegReference(*quantumReg);
        }
        return nullptr;
    }

    iQoalaRegReference *ModuleTranslation::getMappedLocalRegReference(const Value &mlirVal) const {
        if (this->localRegsMap.contains(mlirVal)) {
            return this->localRegsMap.at(mlirVal);
        }
        return nullptr;
    }

    iQoalaRegReference *ModuleTranslation::getMappedQuantumRegReference(const Value &mlirVal) const {
        if (this->quantumRegsMap.contains(mlirVal)) {
            return this->quantumRegsMap.at(mlirVal);
        }
        return nullptr;
    }

    void ModuleTranslation::mapCmpValue(const Value &mlirVal, Operation *mlirOp) {
        const auto result = this->cmpMap.try_emplace(mlirVal, mlirOp);
        (void) result;
        assert(result.second && "Attempting to map a comparison value that is already mapped");
    }

    Operation *ModuleTranslation::getMappedCmpOperation(const Value &mlirVal) const {
        if (this->cmpMap.contains(mlirVal)) {
            return this->cmpMap.at(mlirVal);
        }
        return nullptr;
    }

    iqoala::Block *ModuleTranslation::getMappediQoalaBlock(const mlir::Block *mlirBlock) const {
        assert(this->qoalaHostBlocksMap.contains(mlirBlock) && "original mlirBlock is not mapped");
        return this->qoalaHostBlocksMap.at(mlirBlock);
    }

    // This function inserts NetQASM instructions to comply with the "call conversion" of
    // NetQASM local routines.
    LogicalResult ModuleTranslation::loadClassicalArgWithCallConv(LocalQuantumRoutine *iQoalaRoutine, Operation *localRoutineOp,
        const Value &localRoutineArgVal, const uint8_t argIndex) {
        auto op = dyn_cast<LocalRoutineOp>(localRoutineOp);
        // Immediate with the number of the argument
        iQoalaMCOperand *immediateVal = iQoalaMCOperand::createImmediateOperand(static_cast<uint32_t>(argIndex));
        // Despite this instruction yields a value, we don't need to map it to any
        // mlir value, so we pass "std::nullopt" as the "result" argument when building
        auto *assignInstr = qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
            this, op.getOperation(), NetQASMMCInstr::OP_SET,
            {}, {C}, {immediateVal},
            /*useOpOperands=*/false, /*appendInstruction=*/false, /*mapResults=*/false);
        if (!assignInstr) {
            return failure();
        }
        iQoalaRoutine->addInstruction(assignInstr);

        // Use the "load" instruction to actually load the value
        // Here, the yielded registry needs to be mapped to the mlir value of the function
        // argument so we pass it to the builder for mapping
        // First, get the register reference yielded from the last instruction
        const auto *setResultOperand = assignInstr->getOperand(0);
        assert(setResultOperand->isRegister() && "NetQASM local routine param: result of set is not a register");
        // Create an operand with it
        iQoalaRegReference *setRegRef = iQoalaRegReference::createRegReference(setResultOperand->getRegRef());
        // Pass that extra operand to create the respective load instruction
        auto *loadInstr = iqoala::helpers::buildInstruction<NetQASMMCInstr>(
            this, op.getOperation(), NetQASMMCInstr::OP_LOAD,
            {localRoutineArgVal}, {R}, {iQoalaMCOperand::createRegisterOperand(setRegRef)},
            /*useOpOperands=*/false, /*appendInstruction=*/false);
        if (!loadInstr) {
            return failure();
        }
        iQoalaRoutine->addInstruction(loadInstr);
        return success();
    }

    LogicalResult ModuleTranslation::loadQuantumArgWithCalConv(QuantumRoutine *iQoalaRoutine, Operation *localRoutineOp,
        const Value &qoalaHostQubitVal, const Value &localRoutineArgVal, const uint8_t argIndex) {
        // Follow the iQoala call convertion "for qubits"
        const iQoalaContext *ctx = this->iQoalaModule->getiQoalaContext();
        const uint32_t phyQubitID = ctx->getQubitIDFor(qoalaHostQubitVal);
        LLVM_DEBUG(llvm::dbgs() << "mapping:" << qoalaHostQubitVal << "\n");

        assert(ctx->valueIsMappedToQubit(qoalaHostQubitVal) && "Arg is not mapped to a qubit in the QoalaHost section.");
        LLVM_DEBUG(llvm::dbgs() << "Argument " << static_cast<unsigned>(argIndex) << " is mapped to Qubit as value: " << localRoutineArgVal <<"\n");

        // Insert a "set" instruction in the iQoala local routine, so we can "load" the
        // qubit argument as per the calling convention
        auto *loadInstr = iqoala::helpers::buildInstruction<NetQASMMCInstr>(
            this, localRoutineOp, NetQASMMCInstr::OP_SET,
            {localRoutineArgVal}, {Q}, {iQoalaMCOperand::createImmediateOperand(phyQubitID)},
            /*useOpOperands=*/false, /*appendInstruction=*/false);
        if (!loadInstr) {
            return failure();
        }
        iQoalaRoutine->addInstruction(loadInstr);

        // Register the qubit for the "uses" and "keeps"
        iQoalaRoutine->registerQubit(localRoutineArgVal, phyQubitID);
        return success();
    }


    LogicalResult ModuleTranslation::convertFunctionSignatures() {
        for (auto localRoutine : getModuleBody(this->mlirModule->getOperation()).getOps<LocalRoutineOp>()) {
            if (localRoutine.getName() == helpers::angle::angleConversionFunctionName) {
                // "__qoala_convert_float_angle" is a "routine" of this type
                // Since this routine is intended to be provided by the runtime,
                // we simply don't need to do anything
            } else {
                // We create the routine and process the arguments.
                auto *routine = LocalQuantumRoutine::createLocalRoutine(localRoutine.getName());
                for (auto arg : localRoutine.getArguments()) {
                    // We need to load arguments
                    const uint8_t argNum = arg.getArgNumber();
                    if (!netqasm::blockArgIsQubit(arg)) {
                        // Follow the classical call convention for other args
                        routine->addArgument(helpers::formatString(paramNameFormat, argNum));

                        LLVM_DEBUG(llvm::dbgs() << "Arg " << arg << "\n");
                        if (failed(this->loadClassicalArgWithCallConv(routine, localRoutine.getOperation(), arg, argNum))) {
                            return failure();
                        }
                    }/* else {
                        // For qubit arguments, we will do this when processing the call operations
                        // since we need to allocate physical qubits. Doing it later simplifies the
                        // process to keep track of the allocated physical qubits, and which are
                        // the MLIR values that are mapped to those qubits.
                    }*/
                }
                this->iQoalaModule->addRoutine(routine);
            }
        }
        for (auto requestRoutine : getModuleBody(this->mlirModule->getOperation()).getOps<RequestRoutineOp>()) {
            // We make sure that the request routine returns something, either an i32 (an entangled qubit)
            // or an i1 (a measurement of an entangled qubit)
            FunctionType reqRoutineType = requestRoutine.getFunctionType();
            const auto results = reqRoutineType.getResults();

            if (results.empty()) {
#if __cplusplus >= 202002L
                const std::string errorFmt = "request routine '{}' must return a value";
#else
                const std::string errorFmt = "request routine '%s' must return a value";
#endif
                const std::string errorMessage = helpers::formatString(errorFmt, requestRoutine.getName().data());
                requestRoutine.emitOpError(errorMessage);
                return failure();
            }

            for (auto result : results) {
                if (!(result.isInteger(1) || result.isInteger(32))) {
#if __cplusplus >= 202002L
                    const std::string errorFmt = "request routine '{}' returns an invalid type";
#else
                    const std::string errorFmt = "request routine '%s' returns an invalid type";
#endif
                    const std::string errorMessage = helpers::formatString(errorFmt, requestRoutine.getName().data());
                    requestRoutine.emitOpError(errorMessage);
                    return failure();
                }
            }

            auto *reqRoutine = RequestQuantumRoutine::createRequestRoutine(requestRoutine.getName());

            // We process the arguments of the request routine.
            for (auto argument : requestRoutine.getArguments()) {
                // The arguments *must* be qubit references
                if(!argument.getUses().empty()) {
                    // Request routines do not support using the arguments
                    // We check that, if there are arguments, at least they are not used
#if __cplusplus >= 202002L
                    const std::string errorFmt = "argument #{} from request routine '{}' is used. This is not supported";
#else
                    const std::string errorFmt = "argument #%d from request routine '%s' is used. This is not supported";
#endif
                    const std::string errorMessage = helpers::formatString(
                        errorFmt, argument.getArgNumber(), requestRoutine.getName().data());
                    requestRoutine.emitOpError(errorMessage);
                    return failure();
                }
            }
            this->iQoalaModule->addRoutine(reqRoutine);
        }
        return success();
    }


    std::unique_ptr<iQoalaModule> translateModuleToiQoala(
        Operation *originalModule, iQoalaContext &iQoalaContext, StringRef name) {
        // Entry point for the transformations
        auto iQoalaModule = std::make_unique<iqoala::iQoalaModule>(name, &iQoalaContext);
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

        // We need to explore other module-level operations separately
        // We use getOps here instead of walk for 2 reasons:
        // * We want to explore operations within the region of the module, *not* nested further
        // * We want to proceed using a strict order to translate operations:

        // First, the remote declarations
        for (auto mainFunc : mlirModule.getOps<RemoteOp>()) {
            if (failed(moduleTranslation.convertOperation(*mainFunc.getOperation()))) {
                return nullptr;
            }
        }
        LLVM_DEBUG(llvm::dbgs() << "iQoala after remote translation:\n" << *moduleTranslation.iQoalaModule << "\n********\n");

        // Second, the main function and recursively, all the called routines
        for (auto mainFunc : mlirModule.getOps<MainFuncOp>()) {
            if (failed(moduleTranslation.convertOperation(*mainFunc.getOperation()))) {
                return nullptr;
            }
        }
        // After translating the main function and the called routines, we need to resolve
        // all the instruction references within the local routines
        for (const auto quantumRoutine : moduleTranslation.iQoalaModule->getLocalRoutines()) {
            quantumRoutine->resolveInternalInstrRefs();
        }
        LLVM_DEBUG(llvm::dbgs() << "iQoala after main-func translation:\n" << *moduleTranslation.iQoalaModule << "\n********\n");

        // Third, everything else... that has not been visited yet
        for (Operation &op : getModuleBody(originalModule).getOperations()) {
            // If a local/request routine gets translated here, it means that it is not called
            // from the qoalahost section. Should we consider it as dead code and delete them?
            // Note: We filter out RemoteOps, since they were already translated before
            if (!isa<RemoteOp>(&op) && !iQoalaContext.isOperationVisited(&op) &&
                failed(moduleTranslation.convertOperation(op))) {
                return nullptr;
            }
        }
        LLVM_DEBUG(llvm::dbgs() << "iQoala after other translation:\n" << *moduleTranslation.iQoalaModule << "\n********\n");
        return std::move(moduleTranslation.iQoalaModule);
    }
}
