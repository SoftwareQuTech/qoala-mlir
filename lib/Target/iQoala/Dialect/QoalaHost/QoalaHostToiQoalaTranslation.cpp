#include "llvm/ADT/TypeSwitch.h"
#include "mlir/IR/Operation.h"
#include "Analysis/NetQASM/Helpers.h"
#include "Target/iQoala/ModuleTranslation.h"
#include "Target/iQoala/MC/Helpers.h"
#include "Target/iQoala/MC/iQoalaMC.h"
#include "Target/iQoala/Dialect/QoalaHost/QoalaHostToiQoalaTranslation.h"
#include "Dialect/Helpers/DialectHelpers.h"
#include "Dialect/QoalaHost/QoalaHost.h"

#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "qoalahost-translation"

using namespace mlir;
using namespace qoala::translate;
using namespace qoala::assembly;
using namespace qoala::iqoala;
using namespace qoala::analysis;
using namespace qoala::dialects::helpers;
using namespace qoala::dialects::qoalahost;

static LogicalResult translateBlock(mlir::Block &block, ModuleTranslation *moduleTranslation) {
    for (Operation &op: block.getOperations()) {
        if (failed(moduleTranslation->convertOperation(op))) {
            return op.emitOpError("cannot convert operation '") << op << "'\n";
        }
    }
    return success();
}

static LogicalResult translateMainFunction(MainFuncOp &mainFuncOP, ModuleTranslation *moduleTranslation) {
    moduleTranslation->setModuleName(mainFuncOP.getName());
    // First, we put placeholder (empty) blocks for each one of the basic blocks of the
    for (mlir::Block &block: mainFuncOP.getBlocks()) {
        moduleTranslation->emplaceNewBlockInHostSection(&block);
    }

    // Then, we translate the block and the operations within it
    for (mlir::Block &block: mainFuncOP.getBlocks()) {
        if (failed(translateBlock(block, moduleTranslation))) {
            return mainFuncOP->emitOpError("cannot convert a block inside function '")
                   << mainFuncOP.getSymName() << "'\n";
        }
    }
    return success();
}

static void replacePlaceholderMCOperands(const ModuleTranslation *moduleTranslation, const QuantumRoutine *routine,
                                         const netqasm::ArgValueMap &argValsMap,
                                         const DenseMap<Value, uint32_t> &valueToQubitMap) {
    iQoalaContext *context = moduleTranslation->getQoalaModule()->getiQoalaContext();
    std::string routineName = routine->getName();

    // We need to add mapping for the MLIR values fo the already-existing call convention instructions
    for (uint32_t i = 0; i < routine->getNumInstructions(); i++) {
        if (iQoalaMCInstruction *instruction = routine->getInstruction(i); instruction->hasPlaceholderOperand()) {
            // If the instruction has a placeholder operand, we need to replace it
            BlockArgument routineArgVal = moduleTranslation->getValueForMCInstruction(instruction);
            const uint32_t mappedQubitID = valueToQubitMap.at(argValsMap.getCallerValueForArg(routineArgVal));

            for (uint32_t j = 0; j < instruction->getNumOperands(); j++) {
                if (const iQoalaMCOperand *operand = instruction->getOperand(j); operand->isPlaceHolder()) {
                    // The operand is a placeholder; it needs to be replaced with an immediate operand that
                    // contains the qubit ID of the allocated qubit (either existent or newly allocated)
                    uint32_t qubitID;
                    if (mappedQubitID != 0xFF) {
                        // Qubit is already mapped, used the value
                        qubitID = mappedQubitID;
                    } else {
                        // qubit is not mapped; allocate a new qubit
                        qubitID = context->allocateQubit();
                    }
                    iQoalaMCOperand *newOperand = iQoalaMCOperand::createImmediateOperand(qubitID);
                    instruction->replaceOperand(j, newOperand);
                    // Free the memory for the replaced placeholder
                    delete operand;
                }
            }
        }
    }
}

static LogicalResult processCallToRoutine(ModuleTranslation *moduleTranslation, CallOp &op, const StringRef &callee) {
    const iQoalaModule *iQoalaModule = moduleTranslation->getQoalaModule();
    iQoalaContext *context = iQoalaModule->getiQoalaContext();
    ModuleOp *mlirModule = moduleTranslation->getMLIRModule();
    const std::string calleeStr = callee.str();

    Operation *calledFunction = getRoutineWithName(mlirModule, callee);
    if (!calledFunction) {
        return failure();
    }

    // BEFORE pushing a new frame on the stack, we have to discover the arguments that are mapped to
    // a qubit in the current stack frame:
    DenseMap<Value, uint32_t> valueToQubitMap;
    for (Value arg: op.getOperands()) {
        valueToQubitMap.try_emplace(arg, moduleTranslation->getMappedRegRefForValue(arg, /*copy=*/false)->getQubitID());
    }

    // Some preparations before processing arguments
    // Get the information about the MLIR values (block args) that model the routine args
    // We use this information to map the qubitID within the body of the netqasm local routine
    // so users of the MLIR value inside the MLIR local routine can know that the value is a qubit.
    const netqasm::ArgValueMap valuesArgMap = netqasm::getRoutineArgValues(calledFunction, op.getOperands());
    QuantumRoutine *routine = iQoalaModule->getRoutineByName(callee);
    moduleTranslation->pushNewFrame(calledFunction);
    replacePlaceholderMCOperands(moduleTranslation, routine, valuesArgMap, valueToQubitMap);

    // First, we map the MLIR the values to their regRefs following the call convention
    for (uint32_t argNum = 0; argNum < op.getNumOperands(); ++argNum) {
        const Value valueAtCaller = op.getOperand(argNum);
        const BlockArgument &valueAtCallee = valuesArgMap.getBlockArgForCallerValue(valueAtCaller);

        if (valueToQubitMap[valueAtCaller] != 0xFF) {
            // In this case, the argument is mapped to a qubit reference; search the corresponding set instruction
            // that "loads" the qubit reference
            for (const iQoalaMCInstruction *setInstr:
                 netqasm::filterInstructionsFromRoutine(routine, NetQASMMCInstr::OP_SET)) {
                if (setInstr->getOperand(1)->getIntegerVal() == valueToQubitMap[valueAtCaller]) {
                    moduleTranslation->mapValueToRegRef(valueAtCallee, setInstr->getOperand(0)->getRegRef());
                    // Register the qubit for the "uses" and "keeps"
                    routine->registerQubit(valueAtCallee, setInstr->getOperand(1)->getIntegerVal());
                }
            }
        } else {
            // In this case, we can safely assume that the value is mapped to a classical value.
            for (const iQoalaMCInstruction *loadInstr:
                 netqasm::filterInstructionsFromRoutine(routine, NetQASMMCInstr::OP_LOAD)) {
                if (loadInstr->getOperand(1)->getIntegerVal() == argNum) {
                    moduleTranslation->mapValueToRegRef(valueAtCallee, loadInstr->getOperand(0)->getRegRef());
                }
            }
        }
    }
    // Then we translate the called function and recursively all the nested operations
    if (failed(moduleTranslation->convertOperation(*calledFunction))) {
        return failure();
    }

    // We finally mark the operation as visited, and perform any finalization in the routine body
    context->markOperationAsVisited(calledFunction);
    routine->finalizeRoutine();
    return success();
}

static LogicalResult translateQoalaHostOperation(Operation *operation, ModuleTranslation *moduleTranslation) {
    LLVM_DEBUG(llvm::dbgs() << "******** Translating op '" << operation->getName() << "' *********\n");
    ModuleOp *mlirModule = moduleTranslation->getMLIRModule();
    const iQoalaModule *iQoalaModule = moduleTranslation->getQoalaModule();
    iQoalaContext *context = iQoalaModule->getiQoalaContext();
    return llvm::TypeSwitch<Operation *, LogicalResult>(operation)
            .Case([&](MainFuncOp op) -> LogicalResult {
                moduleTranslation->pushNewFrame(op.getOperation());
                context->markOperationAsVisited(op.getOperation());
                return translateMainFunction(op, moduleTranslation);
            })
            .Case([&](CallOp op) -> LogicalResult {
                // Set the correct opcode depending on the type of the callee
                QoalaHostMCInstr::OpCode opCode = QoalaHostMCInstr::OP_UNKNOWN;
                const StringRef callee = op.getCallee();
                const std::string calleeStr = callee.str();

                if (failed(processCallToRoutine(moduleTranslation, op, callee))) {
                    return failure();
                }

                // Compute the right opcode
                if (hasLocalRoutineWithName(mlirModule, callee)) {
                    opCode = QoalaHostMCInstr::OP_RUN_SUBROUTINE;
                }
                if (hasRequestRoutineWithName(mlirModule, callee)) {
                    opCode = QoalaHostMCInstr::OP_RUN_REQUEST;
                }

                if (opCode == QoalaHostMCInstr::OP_UNKNOWN) {
                    op.emitOpError("Call op: Calling an operation of un unknown type");
                    return failure();
                }

                // The qubit references returned by the local routine *must not* be used as yielded values
                // by this call op. This information will be kept inside the "uses" and "keeps" header section
                // of the local routines
                // In this sense, we need to map the MLIR value yielded by the return to a qubit ID in the
                // qoala section, so we can use this information when calling other routines later
                // Prepare vectors with the results and their local register types
                std::vector<Value> yieldedResults;
                std::vector<iQoalaRegType> localRegTypes;
                for (Value result: op.getResults()) {
                    if (!moduleTranslation->valueIsMappedToQubitInCurrentFrame(result)) {
                        yieldedResults.push_back(result);
                        localRegTypes.push_back(LOCAL);
                    }
                }

                // We also need to check the operands; if the value used as operand is
                // mapped to a qubit, then we don't need to pass it as a routine argument.
                SmallVector<iQoalaMCOperand *> callMCOperands;
                for (const Value callArg: op.getOperands()) {
                    if (!moduleTranslation->valueIsMappedToQubitInCurrentFrame(callArg)) {
                        // If the value is mapped to a qubit, then we don't need to use it
                        // as an operand of the MC instruction
                        iQoalaRegReference *regRef = moduleTranslation->getMappedRegRefForValue(callArg);
                        assert(regRef && "QoalaHost call op builder: operand not mapped");
                        assert(regRef->isLocal() && "QoalaHost call op builder: mapped register is not local");
                        callMCOperands.push_back(iQoalaMCOperand::createRegisterOperand(regRef));
                    }
                }

                // We will set the callee as an "extra" operand, which will be the last of the
                // operands of the MC instruction
                iQoalaMCExpr *calleeSymExpr = iQoalaMCExpr::createSymbolRef(callee.str());
                iQoalaMCOperand *calleeOperand = iQoalaMCOperand::createExprOperand(calleeSymExpr);
                callMCOperands.push_back(calleeOperand);

                // Create qoalahost MC instruction
                const auto *instruction = qoala::iqoala::helpers::buildInstruction<QoalaHostMCInstr>(
                        moduleTranslation, op.getOperation(), opCode, yieldedResults, localRegTypes, callMCOperands,
                        /*useOpOperands=*/false);

                // Then, we need to identify the yielded values that represent a qubit and map them
                // in the qoalahost section to the physical qubitID
                const QuantumRoutine *routine = iQoalaModule->getRoutineByName(callee);

                for (auto &[retIndex, qubitId]: netqasm::getReturnedQubitsMap(mlirModule, callee, routine)) {
                    Value valueAtCaller = op.getResult(retIndex);
                    // We get the register reference for the value, but NOT a copy of the object, since we need
                    // to mutate the original one
                    iQoalaRegReference *regRefCaller =
                            moduleTranslation->getMappedRegRefForValue(valueAtCaller, /*copy=*/false);
                    regRefCaller->setQubitID(qubitId);
                }
                return instruction ? success() : failure();
            })
            .Case([&](ReturnOp op) -> LogicalResult {
                for (const auto returnedValue: op.getOperands()) {
                    iQoalaRegReference *retValRef = moduleTranslation->getMappedRegRefForValue(returnedValue);
                    assert(retValRef && "Return op: trying to return a value which is not mapped to a local registry");
                    iQoalaMCOperand *retValueOperand = iQoalaMCOperand::createRegisterOperand(retValRef);
                    const auto *instruction = qoala::iqoala::helpers::buildInstruction<QoalaHostMCInstr>(
                            moduleTranslation, op.getOperation(), QoalaHostMCInstr::OP_RETURN_RESULT, {}, {},
                            {retValueOperand},
                            /*useOpOperands=*/false);
                    if (!instruction) {
                        op.emitOpError("Return op: could not create return_value instruction");
                        return failure();
                    }
                }
                (void) moduleTranslation->popFrame();
                return success();
            })
            .Case([](SendIntsOp op) -> LogicalResult {
                // TODO - This will be implemented *after* ticket #72, which will implement the lowering of tensors
                return success();
            })
            .Case([](RecvIntsOp op) -> LogicalResult {
                // TODO - This will be implemented *after* ticket #72, which will implement the lowering of tensors
                return success();
            })
            .Case([](NopTOp op) -> LogicalResult {
                // There is nothing to do here
                return success();
            })
            .Case([&](BlkMeta op) -> LogicalResult {
                qoala::iqoala::Block *block = moduleTranslation->getMappediQoalaBlock(op->getBlock());
                // We check if the IDs were already seen. If so, we can add them to the predecessors of the block.
                // Otherwise, we can assume that something is wrong (a block can be defined before its predecessors),
                // and we fail.
                for (StringRef pred: op.getPredecessorsAttr().getAsValueRange<StringAttr>()) {
                    if (auto predecessor = moduleTranslation->findIdPrecedence(pred.str())) {
                        block->addPredecessor(predecessor.value());
                    } else {
                        return failure();
                    }
                }
                for (StringRef dep: op.getDependenciesAttr().getAsValueRange<StringAttr>()) {
                    if (auto dependency = moduleTranslation->findIdPrecedence(dep.str())) {
                        block->addDependency(dependency.value());
                    } else {
                        return failure();
                    }
                }
                std::string prevComm = op.getPrevCommAttr().getValue().str();
                if (!prevComm.empty()) {
                    if (auto blk = moduleTranslation->findIdPrecedence(prevComm)) {
                        block->setPrevComm(blk.value());
                    } else {
                        return failure();
                    }
                } else {
                    block->setPrevComm(nullptr);
                }
                std::string prevEnt = op.getPrevEntAttr().getValue().str();
                if (!prevEnt.empty()) {
                    if (auto blk = moduleTranslation->findIdPrecedence(prevComm)) {
                        block->setPrevEnt(blk.value());
                    } else {
                        return failure();
                    }
                } else {
                    block->setPrevComm(nullptr);
                }

                // We can safely add the new mapping between the dependency ID and the Block.
                moduleTranslation->addIdPrecedence(op.getBlockId().str(), block);
                return success();
            })
            .Case([](const SendFloatsOp op) -> LogicalResult {
                // TODO - This will be implemented *after* ticket #72, which will implement the lowering of tensors
                return op->emitOpError("Sending floats is not supported yet: '") << *op << "'\n";
            })
            .Case([](const RecvFloatsOp op) -> LogicalResult {
                // TODO - This will be implemented *after* ticket #72, which will implement the lowering of tensors
                return op->emitOpError("Receiving floats is not supported yet: '") << *op << "'\n";
            })
            .Default([](Operation *op) -> LogicalResult {
                return op->emitOpError("Unknown way to translate a QoalaHost operation to iQoala: '") << *op << "'\n";
            });
}

namespace qoala::translate {
    LogicalResult QoalaHostToiQoalaTranslation::convertOperation(Operation *op,
                                                                 ModuleTranslation *moduleTranslation) const {
        return translateQoalaHostOperation(op, moduleTranslation);
    }
} // namespace qoala::translate
