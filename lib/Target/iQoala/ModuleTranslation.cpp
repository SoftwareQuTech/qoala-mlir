#include "mlir/IR/BuiltinOps.h"

#include "Analysis/Helpers/Helpers.h"
#include "Conversion/Helpers/Helpers.h"
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
using namespace qoala::dialects::netqasm;

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
        assert(llvm::isa<ModuleOp>(module));
        return module->getRegion(0).front();
    }

    ModuleOp *ModuleTranslation::getMLIRModule() const { return mlirModule; }
    iQoalaModule *ModuleTranslation::getQoalaModule() const { return iQoalaModule.get(); }

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

    void ModuleTranslation::mapValue(const Value &mlirVal, iQoalaRegReference *regRef) {
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

    iQoalaRegReference *ModuleTranslation::getMappedRegReference(const Value &mlirVal) const {
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
    static LogicalResult loadArgument(ModuleTranslation *moduleTranslation, LocalQuantumRoutine *routine, LocalRoutineOp &op,
        Value &mlirArgValue, const uint8_t paramNum) {
        // Immediate with the number of the argument
        iQoalaMCOperand *immediateVal = iQoalaMCOperand::createImmediateOperand(static_cast<uint32_t>(paramNum));
        // Assign that immediate value to a registry
        SmallVector<iQoalaMCOperand *> immediateOperands;
        immediateOperands.push_back(immediateVal);
        // Despite this instruction yields a value, we don't need to map it to any
        // mlir value, so we pass "std::nullopt" as the "result" argument when building
        auto *assignInstr = qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
            moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_SET,
            {}, {C}, immediateOperands,
            /*useOpOperands=*/false, /*appendInstruction=*/false);
        if (!assignInstr) {
            return failure();
        }
        routine->addInstruction(assignInstr);

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
            moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_LOAD,
            {mlirArgValue}, {R}, {iQoalaMCOperand::createRegisterOperand(setRegRef)},
            /*useOpOperands=*/false, /*appendInstruction=*/false);
        if (!loadInstr) {
            return failure();
        }
        routine->addInstruction(loadInstr);
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
                    const uint8_t argNum = arg.getArgNumber();
                    routine->addArgument(helpers::formatString(paramNameFormat, argNum));

                    LLVM_DEBUG(llvm::dbgs() << "Arg " << arg << "\n");
                    if (failed(loadArgument(this, routine, localRoutine, arg, argNum))) {
                        return failure();
                    }
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

            // We simply create the routine. Since request routines do not accept arguments,
            // we don't need to process them
            for (auto argument : requestRoutine.getArguments()) {
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
            auto *routine = RequestQuantumRoutine::createRequestRoutine(requestRoutine.getName());
            const uint8_t phyQubitNum = this->iQoalaModule->getiQoalaContext()->allocateQubit();
            routine->addVirtualIDArg(phyQubitNum);
            this->iQoalaModule->addRoutine(routine);
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

        // TODO - We might need to explore other module-level operations separately

        // Then we explore all the operations in the body
        for (Operation &op : getModuleBody(originalModule).getOperations()) {
            if (failed(moduleTranslation.convertOperation(op))) {
                return nullptr;
            }
        }
        // Finally, we need to resolve all the instruction references within the local routines
        for (const auto quantumRoutine : moduleTranslation.iQoalaModule->getLocalRoutines()) {
            quantumRoutine->resolveInternalInstrRefs();
        }
        return std::move(moduleTranslation.iQoalaModule);
    }
}
