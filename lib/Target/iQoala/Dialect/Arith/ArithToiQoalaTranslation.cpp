#include "mlir/IR/Operation.h"
#include "Dialect/Helpers/DialectHelpers.h"
#include "Target/iQoala/QoalaTranslationInterface.h"
#include "Target/iQoala/MC/iQoalaMC.h"
#include "Target/iQoala/iQoalaContext.h"
#include "Target/iQoala/Dialect/Arith/ArithToiQoalaTranslation.h"

#include "llvm/ADT/TypeSwitch.h"
#include "llvm/Support/Debug.h"
#include "mlir/Dialect/Arith/IR/Arith.h"

#define DEBUG_TYPE "arith-translation"

using namespace mlir;
using namespace qoala::iqoala;
using namespace qoala::assembly;
using namespace qoala::dialects;
using namespace qoala::translate;

static LogicalResult addAssignCValInstr(iQoalaContext *context, ModuleTranslation *moduleTranslation, arith::ConstantIntOp &op) {
    iQoalaMCOperand *immediateVal = iQoalaMCOperand::createImmediateOperand(static_cast<uint32_t>(op.value()));
    const uint8_t regNum = context->allocateHostRegister();
    iQoalaRegReference *regRef = iQoalaRegReference::createRegReference(LOCAL, regNum);
    iQoalaMCOperand *regOperand = iQoalaMCOperand::createRegisterOperand(regRef);
    SmallVector<iQoalaMCOperand *> operands;
    operands.push_back(regOperand);
    operands.push_back(immediateVal);
    const auto newAssign = InstructionBuilder::build<QoalaHostMCInstr>(
        op.getOperation(), QoalaHostMCInstr::OP_ASSIGN_CVAL, operands);
    moduleTranslation->mapValue(op.getResult(), regRef);
    auto *block = moduleTranslation->getMappediQoalaBlock(op->getBlock());
    block->appendInstruction(newAssign);
    return success();
}

static LogicalResult addSetInstr(iQoalaContext *context, ModuleTranslation *moduleTranslation, arith::ConstantIntOp &op) {
    iQoalaMCOperand *immediateVal = iQoalaMCOperand::createImmediateOperand(static_cast<uint32_t>(op.value()));
    const uint8_t regNum = context->allocateCRegister();
    iQoalaRegReference *regRef = iQoalaRegReference::createRegReference(C, regNum);
    iQoalaMCOperand *regOperand = iQoalaMCOperand::createRegisterOperand(regRef);
    SmallVector<iQoalaMCOperand *> operands;
    operands.push_back(regOperand);
    operands.push_back(immediateVal);
    const auto newAssign = InstructionBuilder::build<NetQASMMCInstr>(op.getOperation(),
        NetQASMMCInstr::OP_SET, operands);
    moduleTranslation->mapValue(op.getResult(), regRef);
    const std::string localRoutineName = helpers::getParentNetQASMRoutineName(op.getOperation());
    LocalQuantumRoutine *localRoutine = moduleTranslation->getQoalaModule()->getLocalRoutineByName(localRoutineName);
    localRoutine->addInstruction(newAssign);
    return success();
}

static LogicalResult addQoalaHostAddInstr(iQoalaContext *context, ModuleTranslation *moduleTranslation, arith::AddIOp &op) {
    SmallVector<iQoalaMCOperand *> mcOperands;

    const uint8_t regNum = context->allocateCRegister();
    iQoalaRegReference *resRegRef = iQoalaRegReference::createRegReference(LOCAL, regNum);
    iQoalaMCOperand *resultRegOperand = iQoalaMCOperand::createRegisterOperand(resRegRef);
    mcOperands.push_back(resultRegOperand);

    for (const Value operandVal : op.getOperands()) {
        iQoalaRegReference *regRef = moduleTranslation->getMappedLocalRegReference(operandVal);
        assert(regRef && "Instruction Builder: operand not mapped");
        mcOperands.push_back(iQoalaMCOperand::createRegisterOperand(regRef));
    }

    const auto newAdd = InstructionBuilder::build<QoalaHostMCInstr>(
        op.getOperation(), QoalaHostMCInstr::OP_ADD, mcOperands);
    moduleTranslation->mapValue(op.getResult(), resRegRef);

    auto *block = moduleTranslation->getMappediQoalaBlock(op->getBlock());
    block->appendInstruction(newAdd);
    return success();
}

static LogicalResult addNetQASMAddInstr(iQoalaContext *context, ModuleTranslation *moduleTranslation, arith::AddIOp &op) {
    SmallVector<iQoalaMCOperand *> mcOperands;

    const uint8_t regNum = context->allocateCRegister();
    iQoalaRegReference *resRegRef = iQoalaRegReference::createRegReference(C, regNum);
    iQoalaMCOperand *resultRegOperand = iQoalaMCOperand::createRegisterOperand(resRegRef);
    mcOperands.push_back(resultRegOperand);

    for (const Value operandVal : op.getOperands()) {
        iQoalaRegReference *regRef = moduleTranslation->getMappedQuantumRegReference(operandVal);
        assert(regRef && "NetQASM add instruction: left operand not mapped yet");
        mcOperands.push_back(iQoalaMCOperand::createRegisterOperand(regRef));
    }

    const auto newAssign = InstructionBuilder::build<NetQASMMCInstr>(op.getOperation(),
        NetQASMMCInstr::OP_ADD, mcOperands);

    moduleTranslation->mapValue(op.getResult(), resRegRef);

    const std::string localRoutineName = helpers::getParentNetQASMRoutineName(op.getOperation());
    LocalQuantumRoutine *localRoutine = moduleTranslation->getQoalaModule()->getLocalRoutineByName(localRoutineName);
    localRoutine->addInstruction(newAssign);
    return success();
}

/* Entry point for translating any arith operation */
static LogicalResult translateArithOperation(Operation *operation, ModuleTranslation *moduleTranslation) {
    LLVM_DEBUG(llvm::dbgs() << "******** Translating op '" << operation->getName() << "' *********\n");
    iQoalaContext *context = moduleTranslation->getQoalaModule()->getiQoalaContext();
    return llvm::TypeSwitch<Operation *, LogicalResult>(operation)
            .Case<arith::ConstantIntOp>([&](arith::ConstantIntOp op) -> LogicalResult {
                if (helpers::operationIsInsideMainFunc(operation)) {
                    return addAssignCValInstr(context, moduleTranslation, op);
                }
                if (helpers::operationIsInsideLocalRoutineFunc(operation)) {
                    return addSetInstr(context, moduleTranslation, op);
                }
                return op.emitError("Arith constant operation not in host or netqasm section!") << *op << "\n";
            })
            .Case<arith::CmpIOp>([&](arith::CmpIOp op) -> LogicalResult {
                // TODO - Implement cmp operations
                return success();
            })
            .Case<arith::AddIOp>([&](arith::AddIOp op) -> LogicalResult {
                if (helpers::operationIsInsideMainFunc(operation)) {
                    return addQoalaHostAddInstr(context, moduleTranslation, op);
                }
                if (helpers::operationIsInsideLocalRoutineFunc(operation)) {
                    return addNetQASMAddInstr(context, moduleTranslation, op);
                }
                return success();
            })
            .Case<arith::SubIOp>([&](arith::SubIOp op) -> LogicalResult {
                // TODO - Implement sub operations
                return success();
            })
            .Case<arith::MulIOp>([&](arith::MulIOp op) -> LogicalResult {
                // TODO - Implement mul operations
                return success();
            })
            .Case<arith::DivUIOp>([&](arith::DivUIOp op) -> LogicalResult {
                // TODO - Implement divui operations
                return success();
            })
            .Case<arith::RemUIOp>([&](arith::RemUIOp op) -> LogicalResult {
                // TODO - Implement remui operations
                return success();
            })
            .Default([](Operation *op) -> LogicalResult {
                return op->emitOpError("Unknown way to translate a Arith operation to iQoala: '") << *op << "'\n";
            });
}

namespace qoala::translate {
    LogicalResult ArithToiQoalaTranslation::convertOperation(Operation *op, ModuleTranslation *moduleTranslation) const {
        return translateArithOperation(op, moduleTranslation);
    }
}