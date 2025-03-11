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

static LogicalResult addAssignCValInstr(ModuleTranslation *moduleTranslation, arith::ConstantIntOp &op) {
    SmallVector<iQoalaMCOperand *> mcOperands;
    iQoalaMCOperand *immediateVal = iQoalaMCOperand::createImmediateOperand(static_cast<uint32_t>(op.value()));
    mcOperands.push_back(immediateVal);

    const uint8_t regNumber = moduleTranslation->getQoalaModule()->getiQoalaContext()->allocateRegister(LOCAL);
    iQoalaRegReference *resRegRef = iQoalaRegReference::createRegReference(LOCAL, regNumber);

    const auto newAssign = InstructionBuilder::build<QoalaHostMCInstr>(
        moduleTranslation, op.getOperation(),
        op.getResult(), resRegRef,
        QoalaHostMCInstr::OP_ASSIGN_CVAL, mcOperands);
    return newAssign ? success() : failure();
}

static LogicalResult addSetInstr(ModuleTranslation *moduleTranslation, arith::ConstantIntOp &op) {
    SmallVector<iQoalaMCOperand *> operands;
    iQoalaMCOperand *immediateVal = iQoalaMCOperand::createImmediateOperand(static_cast<uint32_t>(op.value()));
    operands.push_back(immediateVal);

    const uint8_t regNumber = moduleTranslation->getQoalaModule()->getiQoalaContext()->allocateRegister(C);
    iQoalaRegReference *resRegRef = iQoalaRegReference::createRegReference(C, regNumber);

    const auto newAssign = InstructionBuilder::build<NetQASMMCInstr>(
        moduleTranslation,op.getOperation(),
        op.getResult(), resRegRef,
        NetQASMMCInstr::OP_SET, operands);
    return newAssign ? success() : failure();
}

static LogicalResult addQoalaHostAddInstr(ModuleTranslation *moduleTranslation, arith::AddIOp &op) {
    SmallVector<iQoalaMCOperand *> operands;

    const uint8_t regNumber = moduleTranslation->getQoalaModule()->getiQoalaContext()->allocateRegister(LOCAL);
    iQoalaRegReference *resRegRef = iQoalaRegReference::createRegReference(LOCAL, regNumber);

    const auto newAdd = InstructionBuilder::build<QoalaHostMCInstr>(
        moduleTranslation, op.getOperation(),
        op.getResult(), resRegRef,
        QoalaHostMCInstr::OP_ADD, operands);
    return newAdd ? success() : failure();
}

static LogicalResult addNetQASMAddInstr(ModuleTranslation *moduleTranslation, arith::AddIOp &op) {
    SmallVector<iQoalaMCOperand *> operands;

    const uint8_t regNumber = moduleTranslation->getQoalaModule()->getiQoalaContext()->allocateRegister(C);
    iQoalaRegReference *resRegRef = iQoalaRegReference::createRegReference(C, regNumber);

    const auto newAssign = InstructionBuilder::build<NetQASMMCInstr>(moduleTranslation,
        op.getOperation(), op.getResult(), resRegRef,
        NetQASMMCInstr::OP_ADD, operands);
    return newAssign ? success() : failure();
}

/* Entry point for translating any arith operation */
static LogicalResult translateArithOperation(Operation *operation, ModuleTranslation *moduleTranslation) {
    LLVM_DEBUG(llvm::dbgs() << "******** Translating op '" << operation->getName() << "' *********\n");
    return llvm::TypeSwitch<Operation *, LogicalResult>(operation)
            .Case<arith::ConstantIntOp>([&](arith::ConstantIntOp op) -> LogicalResult {
                if (helpers::operationIsInsideMainFunc(operation)) {
                    return addAssignCValInstr(moduleTranslation, op);
                }
                if (helpers::operationIsInsideLocalRoutineFunc(operation)) {
                    return addSetInstr(moduleTranslation, op);
                }
                return op.emitError("Arith constant operation not in host or netqasm section!") << *op << "\n";
            })
            .Case<arith::CmpIOp>([&](arith::CmpIOp op) -> LogicalResult {
                // TODO - Implement cmp operations
                return success();
            })
            .Case<arith::AddIOp>([&](arith::AddIOp op) -> LogicalResult {
                if (helpers::operationIsInsideMainFunc(operation)) {
                    return addQoalaHostAddInstr(moduleTranslation, op);
                }
                if (helpers::operationIsInsideLocalRoutineFunc(operation)) {
                    return addNetQASMAddInstr(moduleTranslation, op);
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