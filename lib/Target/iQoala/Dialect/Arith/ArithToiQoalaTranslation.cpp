#include "mlir/IR/Operation.h"
#include "Dialect/Helpers/DialectHelpers.h"
#include "Target/iQoala/QoalaTranslationInterface.h"
#include "Target/iQoala/MC/iQoalaMC.h"
#include "Target/iQoala/Dialect/Arith/ArithToiQoalaTranslation.h"
#include "Target/iQoala/Dialect/Helpers/Helpers.h"

#include "llvm/ADT/TypeSwitch.h"
#include "llvm/Support/Debug.h"
#include "mlir/Dialect/Arith/IR/Arith.h"

#define DEBUG_TYPE "arith-translation"

using namespace mlir;
using namespace qoala::iqoala;
using namespace qoala::assembly;
using namespace qoala::dialects;
using namespace qoala::translate;

/* Entry point for translating any arith operation */
static LogicalResult translateArithOperation(Operation *operation, ModuleTranslation *moduleTranslation) {
    LLVM_DEBUG(llvm::dbgs() << "******** Translating op '" << operation->getName() << "' *********\n");
    return llvm::TypeSwitch<Operation *, LogicalResult>(operation)
            .Case<arith::ConstantIntOp>([&](arith::ConstantIntOp op) -> LogicalResult {
                iQoalaMCOperand *immediateVal = iQoalaMCOperand::createImmediateOperand(static_cast<uint32_t>(op.value()));
                if (qoala::dialects::helpers::operationIsInsideMainFunc(operation)) {
                    return qoala::iqoala::helpers::buildInstruction<QoalaHostMCInstr>(
                        moduleTranslation, op.getOperation(), QoalaHostMCInstr::OP_ASSIGN_CVAL,
                        op.getResult(), LOCAL, immediateVal);
                }
                if (qoala::dialects::helpers::operationIsInsideLocalRoutineFunc(operation)) {
                    return qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
                        moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_SET,
                        op.getResult(), C, immediateVal
                        );
                }
                return op.emitError("Arith constant operation not in host or netqasm section!") << *op << "\n";
            })
            .Case<arith::CmpIOp>([&](arith::CmpIOp op) -> LogicalResult {
                // TODO - Implement cmp operations
                return success();
            })
            .Case<arith::AddIOp>([&](arith::AddIOp op) -> LogicalResult {
                if (qoala::dialects::helpers::operationIsInsideMainFunc(operation)) {
                    return qoala::iqoala::helpers::buildInstruction<QoalaHostMCInstr>(
                        moduleTranslation, op.getOperation(), QoalaHostMCInstr::OP_ADD,
                        op.getResult(), LOCAL, std::nullopt);
                }
                if (qoala::dialects::helpers::operationIsInsideLocalRoutineFunc(operation)) {
                    return qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
                        moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_ADD,
                        op.getResult(), C, std::nullopt);
                }
                return op.emitError("Arith add operation not in host or netqasm section!") << *op << "\n";
            })
            .Case<arith::SubIOp>([&](arith::SubIOp op) -> LogicalResult {
                if (qoala::dialects::helpers::operationIsInsideMainFunc(operation)) {
                    return qoala::iqoala::helpers::buildInstruction<QoalaHostMCInstr>(
                        moduleTranslation, op.getOperation(), QoalaHostMCInstr::OP_SUBTRACT,
                        op.getResult(), LOCAL, std::nullopt);
                }
                if (qoala::dialects::helpers::operationIsInsideLocalRoutineFunc(operation)) {
                    return qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
                        moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_SUB,
                        op.getResult(), C, std::nullopt);
                }
                return op.emitError("Arith sub operation not in host or netqasm section!") << *op << "\n";
            })
            .Case<arith::MulIOp>([&](arith::MulIOp op) -> LogicalResult {
                if (qoala::dialects::helpers::operationIsInsideMainFunc(operation)) {
                    return qoala::iqoala::helpers::buildInstruction<QoalaHostMCInstr>(
                        moduleTranslation, op.getOperation(), QoalaHostMCInstr::OP_MULTIPLY_CONSTANT,
                        op.getResult(), LOCAL, std::nullopt);
                }
                if (qoala::dialects::helpers::operationIsInsideLocalRoutineFunc(operation)) {
                    return qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
                        moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_MUL,
                        op.getResult(), C, std::nullopt);
                }
                return op.emitError("Arith muli operation not in host or netqasm section!") << *op << "\n";
            })
            .Case<arith::DivUIOp>([&](arith::DivUIOp op) -> LogicalResult {
                if (qoala::dialects::helpers::operationIsInsideLocalRoutineFunc(operation)) {
                    return qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
                        moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_DIV,
                        op.getResult(), C, std::nullopt);
                }
                return op.emitError("Arith divui operation not in netqasm section!") << *op << "\n";
            })
            .Case<arith::RemUIOp>([&](arith::RemUIOp op) -> LogicalResult {
                if (qoala::dialects::helpers::operationIsInsideLocalRoutineFunc(operation)) {
                    return qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
                        moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_REM,
                        op.getResult(), C, std::nullopt);
                }
                return op.emitError("Arith remui operation not in netqasm section!") << *op << "\n";
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