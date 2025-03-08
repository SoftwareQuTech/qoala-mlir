#include "mlir/IR/Operation.h"
#include "Target/iQoala/QoalaTranslationInterface.h"
#include "Target/iQoala/MC/iQoalaMC.h"
#include "Target/iQoala/iQoalaContext.h"
#include "Target/iQoala/Dialect/Arith/ArithToiQoalaTranslation.h"

#include "llvm/ADT/TypeSwitch.h"
#include "llvm/Support/Debug.h"
#include "mlir/Dialect/Arith/IR/Arith.h"

#define DEBUG_TYPE "arith-translation"

using namespace mlir;
using namespace qoala::assembly;

static LogicalResult translateArithOperation(Operation *operation, const qoala::translate::ModuleTranslation *moduleTranslation) {
    // TODO - Implement this dispatcher
    LLVM_DEBUG(llvm::dbgs() << "******** Translating op '" << operation->getName() << "' *********\n");
    return llvm::TypeSwitch<Operation *, LogicalResult>(operation)
            .Case<arith::ConstantIntOp>([&](arith::ConstantIntOp op) -> LogicalResult {
                iQoalaMCOperand *immediateVal = iQoalaMCOperand::createImmediateOperand(static_cast<uint32_t>(op.value()));

                qoala::iqoala::iQoalaContext *context = moduleTranslation->getQoalaModule()->getiQoalaContext();
                const uint8_t regNum = context->allocateRRegister();
                // TODO - In the meantime, we will just allocate R registries
                auto *regRef = iQoalaMCOperand::iQoalaRegReference::createRegReference(R, regNum);
                iQoalaMCOperand *regOperand = iQoalaMCOperand::createRegisterOperand(regRef);

                QoalaHostMCInstr *newAssign = QoalaHostMCInstr::createAssignCValInstr(op.getOperation(), regOperand, immediateVal);

                auto *block = moduleTranslation->getMappediQoalaBlock(op->getBlock());
                block->appendInstruction(newAssign);

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