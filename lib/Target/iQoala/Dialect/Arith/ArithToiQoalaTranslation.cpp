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
    QoalaHostMCInstr *newAssign = QoalaHostMCInstr::createAssignCValInstr(
        op.getOperation(), regOperand, immediateVal
    );
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
    NetQASMMCInstr *newAssign = NetQASMMCInstr::createSetInstruction(
        op.getOperation(), regOperand, immediateVal
    );
    moduleTranslation->mapValue(op.getResult(), regRef);
    const std::string localRoutineName = helpers::getParentNetQASMRoutineName(op.getOperation());
    LocalQuantumRoutine *localRoutine = moduleTranslation->getQoalaModule()->getLocalRoutineByName(localRoutineName);
    localRoutine->addInstruction(newAssign);
    return success();
}

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
                // TODO - Implement add operations
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