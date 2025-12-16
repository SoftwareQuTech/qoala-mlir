#include "mlir/IR/BuiltinOps.h"

#include "Analysis/Helpers/Helpers.h"
#include "Analysis/NetQASM/Helpers.h"
#include "Conversion/Helpers/Helpers.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Target/iQoala/MC/Helpers.h"
#include "Target/iQoala/Module.h"
#include "Target/iQoala/ModuleTranslation.h"
#include "Target/iQoala/QoalaTranslationInterface.h"
#include "Target/iQoala/iQoala.h"

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
#if __cplusplus >= 202002L
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

    std::optional<iqoala::Block *> ModuleTranslation::findIdPrecedence(const StringRef &key) const {
        if (this->precedencesIdsToIQoalaBlocks.contains(key)) {
            return std::optional(precedencesIdsToIQoalaBlocks.at(key));
        }
        return std::nullopt;
    }

    ModuleTranslation::ModuleTranslation(ModuleOp *module, std::unique_ptr<iqoala::iQoalaModule> &iQoalaModule):
        mlirModule(module), iQoalaModule(std::move(iQoalaModule)), ifaces(module->getContext()) { }

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

    void ModuleTranslation::ModuleStackFrame::mapValueInScope(const Value &value, iQoalaRegReference *regRef) {
        const auto result = this->valuesInScope.try_emplace(value, regRef);
        (void) result;
        assert(result.second && "Attempting to map a value that is already mapped");
    }

    bool ModuleTranslation::ModuleStackFrame::isValueInScope(const Value &value) const {
        return this->valuesInScope.contains(value);
    }

    iQoalaRegReference *ModuleTranslation::ModuleStackFrame::getRegReferenceForValue(const Value &value) const {
        if (this->valuesInScope.contains(value)) {
            return this->valuesInScope.at(value);
        }
        return nullptr;
    }

    bool ModuleTranslation::ModuleStackFrame::isModule() const { return isa<ModuleOp>(this->operation); }

    bool ModuleTranslation::ModuleStackFrame::isQoalaHost() const { return isa<MainFuncOp>(this->operation); }

    bool ModuleTranslation::ModuleStackFrame::isLocalRoutine() const { return isa<LocalRoutineOp>(this->operation); }

    bool ModuleTranslation::ModuleStackFrame::isRequestRoutine() const {
        return isa<RequestRoutineOp>(this->operation);
    }

    ModuleTranslation::ModuleStack::~ModuleStack() {
        while (!this->frames.empty()) {
            const ModuleStackFrame *frame = this->frames.top();
            this->frames.pop();
            delete frame;
        }
    }

    void ModuleTranslation::ModuleStack::pushNewStackFrame(Operation *op) {
        // ReSharper disable once CppDFAMemoryLeak - Memory is freed in "ModuleTranslation::ModuleStack::popFrame()"
        // method
        auto *newFrame = new ModuleStackFrame(op);
        this->frames.push(newFrame);
    }

    ModuleTranslation::ModuleStackFrame *ModuleTranslation::ModuleStack::peekFrame() { return this->frames.top(); }

    void ModuleTranslation::ModuleStack::popFrame() {
        const ModuleStackFrame *topFrame = this->frames.top();
        this->frames.pop();
        delete topFrame;
    }

    void ModuleTranslation::ModuleStack::mapValueInCurrentStackFrame(const Value &value, iQoalaRegReference *regRef) {
        ModuleStackFrame *currentFrame = this->frames.top();
        currentFrame->mapValueInScope(value, regRef);
    }

    bool ModuleTranslation::ModuleStack::valueIsMapped(const Value &value) {
        return this->getRegRefForValue(value) != nullptr;
    }

    iQoalaRegReference *ModuleTranslation::ModuleStack::getRegRefForValue(const Value &value) {
        const ModuleStackFrame *currentFrame = this->frames.top();
        return currentFrame->getRegReferenceForValue(value);
    }

    void ModuleTranslation::pushNewFrame(Operation *op) {
        assert((isa<MainFuncOp>(op) || isa<LocalRoutineOp>(op) || isa<RequestRoutineOp>(op)) &&
               "ModuleTranslation: trying to push an operation of an invalid type on the stack");
        this->translationStack.pushNewStackFrame(op);
    }

    Operation *ModuleTranslation::peekFrame() { return this->translationStack.peekFrame()->getOperation(); }

    Operation *ModuleTranslation::popFrame() {
        // Retrieve the top frame and release its memory
        Operation *topFrameOp = this->peekFrame();
        this->translationStack.popFrame();
        return topFrameOp;
    }

    void ModuleTranslation::addRemoteDeclaration(const StringRef remoteName, const bool classicalSocket,
                                                 const bool eprsSocket) const {
        this->iQoalaModule->addRemoteDeclaration(remoteName, classicalSocket, eprsSocket);
    }

    std::optional<uint8_t> ModuleTranslation::getEPRSocketIDForRemote(const StringRef remoteName) const {
        return this->iQoalaModule->getEPRSSocketIDForRemote(remoteName);
    }

    std::optional<uint8_t> ModuleTranslation::getClassicalSocketIDForRemote(const StringRef remoteName) const {
        return this->iQoalaModule->getEPRSSocketIDForRemote(remoteName);
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

    void ModuleTranslation::mapValueToRegRef(const Value &mlirVal, iQoalaRegReference *regRef) {
        const ModuleStackFrame *currentFrame = this->translationStack.peekFrame();
        if (regRef->isLocal()) {
            assert(currentFrame->isQoalaHost() && "Attempting to map a local registry on a non-QoalaHost frame");
        }
        if (regRef->isQuantum()) {
            assert((currentFrame->isLocalRoutine() || currentFrame->isRequestRoutine()) &&
                   "Attempting to map a local registry on a non local/request routine frame");
        }
        LLVM_DEBUG(llvm::dbgs() << "*** Mapping ptr: " << regRef << ", value '" << mlirVal
                                << "' to reference: " << *regRef);
        this->translationStack.mapValueInCurrentStackFrame(mlirVal, regRef);
    }

    bool ModuleTranslation::valueIsMappedInCurrentFrame(const Value &value) {
        return this->translationStack.getRegRefForValue(value);
    }

    bool ModuleTranslation::valueIsMappedToQubitInCurrentFrame(const Value &value) {
        if (const iQoalaRegReference *regRef = this->translationStack.getRegRefForValue(value)) {
            return regRef->representsAQubit();
        }
        return false;
    }

    iQoalaRegReference *ModuleTranslation::getMappedRegRefForValue(const Value &mlirVal, const bool copy) {
        // To ease the de-allocation process, we return copies of the references
        iQoalaRegReference *regRef = this->translationStack.getRegRefForValue(mlirVal);
        LLVM_DEBUG(llvm::dbgs() << "**!!** Value: '" << mlirVal << "' mapped: '" << (regRef ? "true" : "false")
                                << "'\n");
        assert(regRef && "Value is not mapped");
        return copy ? new iQoalaRegReference(*regRef) : regRef;
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

    BlockArgument ModuleTranslation::getValueForMCInstruction(const iQoalaMCInstruction *mcInst) const {
        if (this->mcArgsValsMap.contains(mcInst)) {
            return this->mcArgsValsMap.at(mcInst);
        }
        return nullptr;
    }

    iqoala::Block *ModuleTranslation::getMappediQoalaBlock(const mlir::Block *mlirBlock) const {
        assert(this->qoalaHostBlocksMap.contains(mlirBlock) && "original mlirBlock is not mapped");
        return this->qoalaHostBlocksMap.at(mlirBlock);
    }

    // This function inserts NetQASM instructions to comply with the "call conversion" of
    // NetQASM local routines.
    LogicalResult ModuleTranslation::loadClassicalArgWithCallConv(const BlockArgument &blockArg,
                                                                  LocalQuantumRoutine *iQoalaRoutine,
                                                                  Operation *localRoutineOp, const uint32_t argIndex) {
        auto op = dyn_cast<LocalRoutineOp>(localRoutineOp);
        auto *loadInstr = iqoala::helpers::buildInstruction<NetQASMMCInstr>(
                this, op.getOperation(), NetQASMMCInstr::OP_LOAD, {}, {R},
                {iQoalaMCOperand::createImmediateOperand(argIndex)},
                /*useOpOperands=*/false, /*appendInstruction=*/false, /*mapResults=*/false);
        if (!loadInstr) {
            return failure();
        }
        iQoalaRoutine->addInstruction(loadInstr);
        this->mcArgsValsMap.try_emplace(loadInstr, blockArg);
        return success();
    }

    LogicalResult ModuleTranslation::loadQuantumArgWithCalConv(const BlockArgument &blockArg,
                                                               QuantumRoutine *iQoalaRoutine,
                                                               Operation *localRoutineOp) {
        auto *setInstr = iqoala::helpers::buildInstruction<NetQASMMCInstr>(
                this, localRoutineOp, NetQASMMCInstr::OP_SET, {}, {Q}, {iQoalaMCOperand::createPlaceholderOperand()},
                /*useOpOperands=*/false, /*appendInstruction=*/false, /*mapResults=*/false);
        if (!setInstr) {
            return failure();
        }
        iQoalaRoutine->addInstruction(setInstr);
        this->mcArgsValsMap.try_emplace(setInstr, blockArg);
        return success();
    }

    LogicalResult ModuleTranslation::convertLocalRoutines() {
        for (auto localRoutine : getModuleBody(this->mlirModule->getOperation()).getOps<LocalRoutineOp>()) {
            if (localRoutine.getName() == helpers::angle::angleConversionFunctionName) {
                // "__qoala_convert_float_angle" is a "routine" of this type
                // Since this routine is intended to be provided by the runtime,
                // we simply don't need to do anything
            } else {
                // We create the routine and process the arguments.
                auto *routine = LocalQuantumRoutine::createLocalRoutine(localRoutine.getName());
                std::string routineName = localRoutine.getName().str();
                for (auto arg : localRoutine.getArguments()) {
                    // We need to load arguments
                    const uint32_t argNum = arg.getArgNumber();
                    if (netqasm::blockArgIsQubit(arg)) {
                        if (failed(this->loadQuantumArgWithCalConv(arg, routine, localRoutine.getOperation()))) {
                            return failure();
                        }
                    } else {
                        // Follow the classical call convention for other args
                        routine->addArgument(helpers::formatString(paramNameFormat, argNum));
                        if (failed(this->loadClassicalArgWithCallConv(arg, routine, localRoutine.getOperation(),
                                                                      argNum))) {
                            return failure();
                        }
                    }
                }
                this->iQoalaModule->addRoutine(routine);
            }
        }
        return success();
    }

    LogicalResult ModuleTranslation::convertRequestRoutines() const {
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
                if (!argument.getUses().empty()) {
                    // Request routines do not support using the arguments
                    // We check that, if there are arguments, at least they are not used
#if __cplusplus >= 202002L
                    const std::string errorFmt =
                            "argument #{} from request routine '{}' is used. This is not supported";
#else
                    const std::string errorFmt =
                            "argument #%d from request routine '%s' is used. This is not supported";
#endif
                    const std::string errorMessage =
                            helpers::formatString(errorFmt, argument.getArgNumber(), requestRoutine.getName().data());
                    requestRoutine.emitOpError(errorMessage);
                    return failure();
                }
            }
            this->iQoalaModule->addRoutine(reqRoutine);
        }
        return success();
    }

    LogicalResult ModuleTranslation::convertFunctionSignatures() {
        if (failed(this->convertLocalRoutines()) || failed(this->convertRequestRoutines())) {
            return failure();
        }
        return success();
    }

    std::unique_ptr<iQoalaModule> translateModuleToiQoala(Operation *originalModule, iQoalaContext &iQoalaContext,
                                                          StringRef name) {
        // Entry point for the transformations
        auto iQoalaModule = std::make_unique<iqoala::iQoalaModule>(name, &iQoalaContext);
        auto mlirModule = dyn_cast<ModuleOp>(originalModule);
        ModuleTranslation moduleTranslation(&mlirModule, iQoalaModule);
        // First, we translate the module itself
        LLVM_DEBUG(llvm::dbgs() << "******** Translating module '" << originalModule->getName() << "' *********\n");
        LLVM_DEBUG(originalModule->dump());
        if (failed(moduleTranslation.convertOperation(*originalModule))) {
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
        LLVM_DEBUG(llvm::dbgs() << "iQoala after remote translation:\n"
                                << *moduleTranslation.iQoalaModule << "\n********\n");

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
        LLVM_DEBUG(llvm::dbgs() << "iQoala after main-func translation:\n"
                                << *moduleTranslation.iQoalaModule << "\n********\n");

        moduleTranslation.iQoalaModule->deleteEmptyHostBlocks();
        if (failed(moduleTranslation.iQoalaModule->setQoalaHostBlockTypes())) {
            originalModule->emitError() << "Translation yielded a QoalaHost block of type QC, QL or CC "
                                           "that does not match the required number of instructions";
            return nullptr;
        }

        LLVM_DEBUG(llvm::dbgs() << "iQoala after deleting empty blocks:\n"
                                << *moduleTranslation.iQoalaModule << "\n********\n");

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
        LLVM_DEBUG(llvm::dbgs() << "iQoala after other translation:\n"
                                << *moduleTranslation.iQoalaModule << "\n********\n");
        return std::move(moduleTranslation.iQoalaModule);
    }
} // namespace qoala::translate
