#include "mlir/IR/Operation.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "llvm/ADT/TypeSwitch.h"
#include "Dialect/Helpers/DialectHelpers.h"
#include "Target/iQoala/MC/iQoalaMC.h"
#include "Target/iQoala/Dialect/Arith/ArithToiQoalaTranslation.h"
#include "Target/iQoala/MC/Helpers.h"

#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "arith-translation"

using namespace mlir;
using namespace qoala::iqoala;
using namespace qoala::assembly;
using namespace qoala::dialects;
using namespace qoala::translate;

static LogicalResult processOperandsForMul(
    const ModuleTranslation *moduleTranslation, arith::MulIOp &mulOp,
    SmallVector<iQoalaMCOperand *> &mappedOperands) {
    // Due to the folding of instructions when lowering from MIR to LIR, we can safely assume that
    // one of the operands *has to be* the value of an arith.constant operation.
    // We will manually map the other operand to its register reference and iQoalaOperand
    assert(mulOp.getOperands().size() == 2 && "Arith mul: instruction does not have correct number of operands");
    const Value operandA = mulOp.getOperand(0);
    const Value operandB = mulOp.getOperand(0);
    if (auto constOp = dyn_cast<arith::ConstantIntOp>(operandA.getDefiningOp())) {
        iQoalaMCOperand *immediateVal = iQoalaMCOperand::createImmediateOperand(static_cast<uint32_t>(constOp.value()));
        iQoalaRegReference *regRef = moduleTranslation->getMappedRegReference(operandB);
        assert(regRef && "Operands processor for mul: operand not mapped");
        assert(regRef->isLocal() && "Operands processor for mul: mapped register is not quantum");
        iQoalaMCOperand *otherMulOp = iQoalaMCOperand::createRegisterOperand(regRef);
        mappedOperands.push_back(otherMulOp);
        mappedOperands.push_back(immediateVal);
        return success();
    }
    if (auto constOp = dyn_cast<arith::ConstantIntOp>(operandB.getDefiningOp())) {
        iQoalaMCOperand *immediateVal = iQoalaMCOperand::createImmediateOperand(static_cast<uint32_t>(constOp.value()));
        iQoalaRegReference *regRef = moduleTranslation->getMappedRegReference(operandA);
        assert(regRef && "Operands processor for mul: operand not mapped");
        assert(regRef->isLocal() && "Operands processor for mul: mapped register is not quantum");
        iQoalaMCOperand *otherMulOp = iQoalaMCOperand::createRegisterOperand(regRef);
        mappedOperands.push_back(otherMulOp);
        mappedOperands.push_back(immediateVal);
        return success();
    }
    return failure();
}

/* Entry point for translating any arith operation */
static LogicalResult translateArithOperation(Operation *operation, ModuleTranslation *moduleTranslation) {
    LLVM_DEBUG(llvm::dbgs() << "******** Translating op '" << operation->getName() << "' *********\n");
    return llvm::TypeSwitch<Operation *, LogicalResult>(operation)
            .Case<arith::ConstantIntOp>([&](arith::ConstantIntOp op) -> LogicalResult {
                SmallVector<iQoalaMCOperand *> processedOperands;
                iQoalaMCOperand *immediateVal = iQoalaMCOperand::createImmediateOperand(static_cast<uint32_t>(op.value()));
                processedOperands.push_back(immediateVal);
                if (qoala::dialects::helpers::operationIsInsideMainFunc(operation)) {
                    const auto *instruction = qoala::iqoala::helpers::buildInstruction<QoalaHostMCInstr>(
                        moduleTranslation, op.getOperation(), QoalaHostMCInstr::OP_ASSIGN_CVAL,
                        {op.getResult()}, {LOCAL}, processedOperands);
                    return instruction ? success() : failure();
                }
                if (qoala::dialects::helpers::operationIsInsideLocalRoutineFunc(operation)) {
                    const auto *instruction = qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
                        moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_SET,
                        {op.getResult()}, {C}, processedOperands
                        );
                    return instruction ? success() : failure();
                }
                return op.emitOpError("Arith constant operation not in host or netqasm section!\n");
            })
            .Case<arith::CmpIOp>([&](arith::CmpIOp op) -> LogicalResult {
                // In this case, we simply map the operation to the MLIR value yielded.
                // We will use this later when encountering the cf.cond_br instruction to select
                // the right branching instruction to emplace in the QoalalHost/NetQASM block.
                moduleTranslation->mapCmpValue(op.getResult(), op.getOperation());
                return success();
            })
            .Case<arith::AddIOp>([&](arith::AddIOp op) -> LogicalResult {
                if (qoala::dialects::helpers::operationIsInsideMainFunc(operation)) {
                    const auto *instruction = qoala::iqoala::helpers::buildInstruction<QoalaHostMCInstr>(
                        moduleTranslation, op.getOperation(), QoalaHostMCInstr::OP_ADD,
                        {op.getResult()}, {LOCAL});
                    return instruction ? success() : failure();
                }
                if (qoala::dialects::helpers::operationIsInsideLocalRoutineFunc(operation)) {
                    const auto *instruction = qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
                        moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_ADD,
                        {op.getResult()}, {C});
                    return instruction ? success() : failure();
                }
                return op.emitOpError("Arith add operation not in host or netqasm section!\n");
            })
            .Case<arith::SubIOp>([&](arith::SubIOp op) -> LogicalResult {
                if (qoala::dialects::helpers::operationIsInsideMainFunc(operation)) {
                    const auto *instruction = qoala::iqoala::helpers::buildInstruction<QoalaHostMCInstr>(
                        moduleTranslation, op.getOperation(), QoalaHostMCInstr::OP_SUBTRACT,
                        {op.getResult()}, {LOCAL});
                    return instruction ? success() : failure();
                }
                if (qoala::dialects::helpers::operationIsInsideLocalRoutineFunc(operation)) {
                    const auto *instruction = qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
                        moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_SUB,
                        {op.getResult()}, {C});
                    return instruction ? success() : failure();
                }
                return op.emitOpError("Arith sub operation not in host or netqasm section!\n");
            })
            .Case<arith::MulIOp>([&](arith::MulIOp op) -> LogicalResult {
                SmallVector<iQoalaMCOperand *> processedOperands;
                if (qoala::dialects::helpers::operationIsInsideMainFunc(operation)) {
                    // In QoalaHost, multiplication uses an immediate; we need "trace" the immediate back, and create
                    // the immediate value.
                    if (failed(processOperandsForMul(moduleTranslation, op, processedOperands))) {
                        op.emitOpError("Arith mul operation: cannot process operands correctly!");
                    }
                    const auto *instruction = qoala::iqoala::helpers::buildInstruction<QoalaHostMCInstr>(
                        moduleTranslation, op.getOperation(), QoalaHostMCInstr::OP_MULTIPLY_CONSTANT,
                        {op.getResult()}, {LOCAL}, processedOperands, /*useOpOperands=*/false);
                    return instruction ? success() : failure();
                }
                if (qoala::dialects::helpers::operationIsInsideLocalRoutineFunc(operation)) {
                    const auto *instruction = qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
                        moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_MUL,
                        {op.getResult()}, {C}, processedOperands);
                    return instruction ? success() : failure();
                }
                return op.emitOpError("Arith muli operation not in host or netqasm section!\n");
            })
            .Case<arith::DivUIOp>([&](arith::DivUIOp op) -> LogicalResult {
                if (qoala::dialects::helpers::operationIsInsideLocalRoutineFunc(operation)) {
                    const auto *instruction = qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
                        moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_DIV,
                        {op.getResult()}, {C});
                    return instruction ? success() : failure();
                }
                return op.emitOpError("Arith divui operation not in netqasm section!\n");
            })
            .Case<arith::RemUIOp>([&](arith::RemUIOp op) -> LogicalResult {
                if (qoala::dialects::helpers::operationIsInsideLocalRoutineFunc(operation)) {
                    const auto *instruction = qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
                        moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_REM,
                        {op.getResult()}, {C});
                    return instruction ? success() : failure();
                }
                return op.emitOpError("Arith remui operation not in netqasm section!");
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