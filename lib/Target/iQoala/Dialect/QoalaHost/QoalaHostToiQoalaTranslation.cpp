#include "llvm/ADT/TypeSwitch.h"
#include "mlir/IR/Operation.h"
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
    // TODO - Implement this dispatcher
    LLVM_DEBUG(llvm::dbgs() << "******** Translating op '" << operation->getName() << "' *********\n");
    return llvm::TypeSwitch<Operation *, LogicalResult>(operation)
            .Case([&](MainFuncOp op) -> LogicalResult {
                return translateMainFunction(op, moduleTranslation);
            })
            .Case([&](CallOp op) -> LogicalResult {
                // Prepare vectors with the results and their local register types
                std::vector<Value> yieldedResults;
                std::vector<iQoalaRegType> localRegTypes;
                for (Value result : op.getResults()) {
                    yieldedResults.push_back(result);
                    localRegTypes.push_back(LOCAL);
                }

                // Set the correct opcode depending on the type of the callee
                QoalaHostMCInstr::OpCode opCode = QoalaHostMCInstr::OP_UNKNOWN;
                const StringRef callee = op.getCallee();
                if (moduleTranslation->getQoalaModule()->hasLocalRoutineWithName(callee)) {
                    LocalQuantumRoutine *routine = moduleTranslation->getQoalaModule()->getLocalRoutineByName(callee);
                    opCode = QoalaHostMCInstr::OP_RUN_SUBROUTINE;
                }
                if (moduleTranslation->getQoalaModule()->hasRequestRoutineWithName(callee)) {
                    RequestQuantumRoutine *routine = moduleTranslation->getQoalaModule()->getRequestRoutineByName(callee);
                    opCode = QoalaHostMCInstr::OP_RUN_REQUEST;
                }

                if (opCode == QoalaHostMCInstr::OP_UNKNOWN) {
                    op.emitOpError("Call op: Calling an operation of un unknown type");
                    return failure();
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