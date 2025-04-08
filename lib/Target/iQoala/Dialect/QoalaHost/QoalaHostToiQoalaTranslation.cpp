#include "llvm/ADT/TypeSwitch.h"
#include "mlir/IR/Operation.h"
#include "Analysis/NetQASM/Helpers.h"
#include "Dialect/Helpers/DialectHelpers.h"
#include "Target/iQoala/ModuleTranslation.h"
#include "Target/iQoala/MC/Helpers.h"
#include "Target/iQoala/MC/iQoalaMC.h"
#include "Target/iQoala/Dialect/QoalaHost/QoalaHostToiQoalaTranslation.h"
#include "Dialect/QoalaHost/QoalaHost.h"

#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "qoalahost-translation"

using namespace mlir;
using namespace qoala::translate;
using namespace qoala::assembly;
using namespace qoala::iqoala;
using namespace qoala::analysis;
using namespace qoala::dialects::qoalahost;

static LogicalResult translateBlock(mlir::Block &block, ModuleTranslation *moduleTranslation) {
    for (Operation &op : block.getOperations()) {
        if (failed(moduleTranslation->convertOperation(op))) {
            return op.emitOpError("cannot convert operation '") << op << "'\n";
        }
    }
    return success();
}

static LogicalResult translateMainFunction(MainFuncOp &mainFuncOP, ModuleTranslation *moduleTranslation) {
    moduleTranslation->setModuleName(mainFuncOP.getName());
    // First, we put placeholder (empty) blocks for each one of the basic blocks of the
    for (mlir::Block &block : mainFuncOP.getBlocks()) {
        moduleTranslation->emplaceNewBlockInHostSection(&block);
    }

    // Then, we translate the block and the operations within it
    for (mlir::Block &block : mainFuncOP.getBlocks()) {
        if (failed(translateBlock(block, moduleTranslation))) {
            return mainFuncOP->emitOpError("cannot convert a block inside function '")
                    << mainFuncOP.getSymName() << "'\n";
        }
    }
    return success();
}

static LogicalResult translateQoalaHostOperation(Operation *operation, ModuleTranslation *moduleTranslation) {
    LLVM_DEBUG(llvm::dbgs() << "******** Translating op '" << operation->getName() << "' *********\n");
    ModuleOp *mlirModule = moduleTranslation->getMLIRModule();
    iQoalaModule *iQoalaModule = moduleTranslation->getQoalaModule();
    iQoalaContext *context = iQoalaModule->getiQoalaContext();
    return llvm::TypeSwitch<Operation *, LogicalResult>(operation)
            .Case([&](MainFuncOp op) -> LogicalResult {
                return translateMainFunction(op, moduleTranslation);
            })
            .Case([&](CallOp op) -> LogicalResult {
                // Set the correct opcode depending on the type of the callee
                QoalaHostMCInstr::OpCode opCode = QoalaHostMCInstr::OP_UNKNOWN;
                const StringRef callee = op.getCallee();

                if (Operation *calledFunction = netqasm::getLocalRoutineWithName(mlirModule, callee)) {
                    // First we map the arguments to the local MLIR values.
                    // TODO - Move this logic to moduletranslation
                    // For the arguments, values that are already mapped to physical qubit IDs, cannot
                    // be used as arguments in iQoala. We need to pass this information as a "uses" value
                    // in the routine header.
                    LocalQuantumRoutine *localRoutine = iQoalaModule->getLocalRoutineByName(callee);
                    std::map<uint32_t, Value> localRoutineArgs = netqasm::getRoutineArgValues(calledFunction);

                    for (uint32_t argNum = 0;  argNum < op.getNumOperands(); argNum++) {
                        const auto argOperand = op.getOperand(argNum);
                        if (context->valueIsMappedToQubit(argOperand)) {
                            LLVM_DEBUG(llvm::dbgs() << "Argument " << argNum << " is mapped to Qubit as value: " << localRoutineArgs.at(argNum) <<"\n");
                            // TODO - Insert some instructions in the iQoala local routine, so we can "load" the
                            //  qubit argument as per the calling convention
                            // TODO - Create a RegReference for the qubit loaded in the iQoala local routine
                            // TODO - Map the internal local value to the RegReference of the convention call loaded qubit
                            // Register the qubit for the "uses" and "keeps"
                            localRoutine->registerQubit(localRoutineArgs.at(argNum), context->getQubitIDFor(argOperand));
                        }
                    }

                    // Then we translate the called function
                    if (failed(moduleTranslation->convertOperation(*calledFunction))) {
                        return failure();
                    }

                    // Then, we need to identify the yielded values that represent a qubit and map them
                    // in the qoalahost section to the physical qubitID
                    const std::map<uint32_t, uint8_t> returnedQubitsMap = netqasm::getReturnedQubitsMap(mlirModule, callee, localRoutine);

                    for (auto &[retIndex, qubitId] : returnedQubitsMap) {
                        const Value retValue = op.getResult(retIndex);
                        context->mapValueToQubitID(retValue, qubitId);
                    }
                    opCode = QoalaHostMCInstr::OP_RUN_SUBROUTINE;
                }
                if (Operation *calledFunction = netqasm::getRequestRoutineWithName(mlirModule, callee)) {
                    // First of all, we translate the called function
                    if (failed(moduleTranslation->convertOperation(*calledFunction))) {
                        return failure();
                    }
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
                for (Value result : op.getResults()) {
                    if (!context->valueIsMappedToQubit(result)) {
                        yieldedResults.push_back(result);
                        localRegTypes.push_back(LOCAL);
                    }
                }

                // We will set the callee as an "extra" operand, which will be the last of the
                // operands of the MC instruction
                iQoalaMCExpr *calleeSymExpr = iQoalaMCExpr::createSymbolRef(callee.str());
                iQoalaMCOperand *calleeOperand = iQoalaMCOperand::createExprOperand(calleeSymExpr);

                // Create qoalahost MC instruction
                const auto *instruction = qoala::iqoala::helpers::buildInstruction<QoalaHostMCInstr>(
                    moduleTranslation, op.getOperation(), opCode,
                    yieldedResults, localRegTypes , {calleeOperand}
                );
                return instruction ? success() : failure();
            })
            .Case([&](ReturnOp op) -> LogicalResult {
                for (const auto returnedValue :op.getOperands()) {
                    iQoalaRegReference *retValRef = moduleTranslation->getMappedRegReference(returnedValue);
                    assert(retValRef && "Return op: trying to return a value which is not mapped to a local registry");
                    iQoalaMCOperand *retValueOperand = iQoalaMCOperand::createRegisterOperand(retValRef);
                    const auto *instruction = qoala::iqoala::helpers::buildInstruction<QoalaHostMCInstr>(
                        moduleTranslation, op.getOperation(), QoalaHostMCInstr::OP_RETURN_RESULT,
                        {}, {} , {retValueOperand},
                        /*useOpOperands=*/false
                    );
                    if (!instruction) {
                        op.emitOpError("Return op: could not create return_value instruction");
                        return failure();
                    }
                }
                return success();
            })
            .Case([](SendIntsOp op) -> LogicalResult {
                return success();
            })
            .Case([](RecvIntsOp op) -> LogicalResult {
                return success();
            })
            .Case([](NopTOp op) -> LogicalResult {
                // There is nothing to do here
                return success();
            })
            .Case([](BlkMeta op) -> LogicalResult {
                // There is nothing to do here
                return success();
            })
            .Case([](const SendFloatsOp op) -> LogicalResult {
                return op->emitOpError("Sending floats is not supported yet: '") << *op << "'\n";
            })
            .Case([](const RecvFloatsOp op) -> LogicalResult {
                return op->emitOpError("Receiving floats is not supported yet: '") << *op << "'\n";
            })
            .Default([](Operation *op) -> LogicalResult {
                return op->emitOpError("Unknown way to translate a QoalaHost operation to iQoala: '") << *op << "'\n";
            });
}

namespace qoala::translate {
    LogicalResult QoalaHostToiQoalaTranslation::convertOperation(Operation *op, ModuleTranslation *moduleTranslation) const {
        return translateQoalaHostOperation(op, moduleTranslation);
    }
}