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
using namespace qoala::assembly;

static LogicalResult translateArithOperation(Operation *operation, qoala::translate::ModuleTranslation *moduleTranslation) {
    // TODO - Implement this dispatcher
    LLVM_DEBUG(llvm::dbgs() << "******** Translating op '" << operation->getName() << "' *********\n");
    return llvm::TypeSwitch<Operation *, LogicalResult>(operation)
            .Case<arith::ConstantIntOp>([&](arith::ConstantIntOp op) -> LogicalResult {
                iQoalaMCOperand *immediateVal = iQoalaMCOperand::createImmediateOperand(static_cast<uint32_t>(op.value()));

                qoala::iqoala::iQoalaContext *context = moduleTranslation->getQoalaModule()->getiQoalaContext();
                const uint8_t regNum = context->allocateHostRegister();
                iQoalaRegReference *regRef = iQoalaRegReference::createRegReference(LOCAL, regNum);
                iQoalaMCOperand *regOperand = iQoalaMCOperand::createRegisterOperand(regRef);

                if (qoala::dialects::helpers::operationIsInsideMainFunc(operation)) {
                    QoalaHostMCInstr *newAssign = QoalaHostMCInstr::createAssignCValInstr(
                        op.getOperation(), regOperand, immediateVal
                    );
                    moduleTranslation->mapValue(op.getResult(), regRef);
                    auto *block = moduleTranslation->getMappediQoalaBlock(op->getBlock());
                    block->appendInstruction(newAssign);
                } else {
                    if (qoala::dialects::helpers::operationIsInsideLocalRoutineFunc(operation)) {
                        NetQASMMCInstr *newAssign = NetQASMMCInstr::createSetInstruction(
                            op.getOperation(), regOperand, immediateVal
                        );
                        moduleTranslation->mapValue(op.getResult(), regRef);
                        // TODO - Get the name of the netqasm local routine that the original op belongs to
                        // TODO - Get the netqasm section of the  local routine that matches the name
                        // TODO - Append the newAssign operation to the respective netqasm block
                    } else {
                        return op.emitError("Arith constant operation not in host or netqasm section!") << *op << "\n";
                    }
                }
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