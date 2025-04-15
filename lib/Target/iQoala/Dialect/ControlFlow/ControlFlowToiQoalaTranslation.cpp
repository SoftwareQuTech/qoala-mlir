#include "llvm/ADT/TypeSwitch.h"
#include "mlir/IR/Operation.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "Dialect/Helpers/DialectHelpers.h"
#include "Target/iQoala/MC/Helpers.h"
#include "Target/iQoala/MC/iQoalaMC.h"
#include "Target/iQoala/Dialect/ControlFlow/ControlFlowToiQoalaTranslation.h"

#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "builtin-translation"

using namespace mlir;
using namespace qoala::assembly;
using namespace qoala::translate;

static NetQASMMCInstr::OpCode getNetQASMOpCodeFor(const arith::CmpIPredicate predicate, const bool operandIsZero) {
    // Mapping between the arith.cmpi predicate values to the respective NetQASM Opcodes
    switch (predicate) {
        case arith::CmpIPredicate::eq:
            return operandIsZero ? NetQASMMCInstr:: OP_BEZ : NetQASMMCInstr::OP_BEQ;
        case arith::CmpIPredicate::ne:
            return operandIsZero ? NetQASMMCInstr::OP_BNZ : NetQASMMCInstr::OP_BNE;
        case arith::CmpIPredicate::sge:
        case arith::CmpIPredicate::uge:
            return NetQASMMCInstr::OP_BGE;
        case arith::CmpIPredicate::slt:
        case arith::CmpIPredicate::ult:
            return NetQASMMCInstr::OP_BLT;
        default:
            return NetQASMMCInstr::OP_UNKNOWN;
    }
}

static QoalaHostMCInstr::OpCode getQoalaHostOpCodeFor(const arith::CmpIPredicate predicate) {
    // Mapping between the arith.cmpi predicate values to the respective QoalaHost Opcodes
    switch (predicate) {
        case arith::CmpIPredicate::eq:
            return QoalaHostMCInstr::OP_BEQ;
        case arith::CmpIPredicate::ne:
            return QoalaHostMCInstr::OP_BNE;
        case arith::CmpIPredicate::sgt:
        case arith::CmpIPredicate::ugt:
            return QoalaHostMCInstr::OP_BGT;
        case arith::CmpIPredicate::slt:
        case arith::CmpIPredicate::ult:
            return QoalaHostMCInstr::OP_BLT;
        default:
            return QoalaHostMCInstr::OP_UNKNOWN;
    }
}

static LogicalResult placeQoalaHostJumpInstr(ModuleTranslation *moduleTranslation, cf::BranchOp &op) {
    // Get the name of the target block
    const Block *destBlock = op.getDest();
    const auto *destQoalaHostBlock = moduleTranslation->getMappediQoalaBlock(destBlock);
    assert(destQoalaHostBlock && "Destination block not mapped!");
    const std::string targetBlockName = destQoalaHostBlock->getName();

    // Use the target name as an operand
    iQoalaMCExpr *blockSymExpr = iQoalaMCExpr::createSymbolRef(targetBlockName);
    iQoalaMCOperand *targetBlockOperand = iQoalaMCOperand::createExprOperand(blockSymExpr);

    // Create the jump instruction
    const auto *instruction = qoala::iqoala::helpers::buildInstruction<QoalaHostMCInstr>(
        moduleTranslation, op.getOperation(), QoalaHostMCInstr::OP_JUMP,
        {}, {}, {targetBlockOperand},
        /*useOpOperands=*/false, /*appendInstruction=*/true);
    return instruction ? success() : failure();
}

static LogicalResult placeQoalaHostCondBrInstr(ModuleTranslation *moduleTranslation, cf::CondBranchOp &op) {
    // In MLIR, conditional branch instructions are semantically different (using an eq comparison as example):
    // %test = arith.cmpi eq %op1, %op2 : i32
    // cf.cond_br %test, <dest_if_true>, <dest_if_false>
    // This means that they might jump to different locations depending on whether the test is true or false.
    // On the other side, in QoalaHost, branching instructions jump *only* if the condition is true:
    // beq(%op1, %op2) : <dest_if_true>
    // if the test is false (%op1 != %op2 in this case), the control will simply execute the next instruction.
    // To solve this issue, we need to consider a few things:
    // * In MLIR, branching instructions are block terminators, meaning that it is the last instruction of a block
    // * Despite there is no such restriction in QoalaHost, it is safe to assume the same for a QoalaHost block
    // A way to implement the same MLIR semantics in QoalaHost is to place an unconditional jump right after
    // the conditional jump. In this way, the MLIR combo:
    // %test = arith.cmpi eq %op1, %op2 : i32
    // cf.cond_br %test, <dest_if_true>, <dest_if_false>
    // would be mapped to:
    // beq (%op1, %op2) : <dest_if_true>
    // jump () : <dest_if_false>

    const Value condition = op.getCondition();
    Operation *cmpOp = moduleTranslation->getMappedCmpOperation(condition);
    if (auto cmpIOp = dyn_cast<arith::CmpIOp>(cmpOp)) {
        // Process the destinations of the MLIR conditional jump
        const Block *trueDestBlock = op.getTrueDest();
        const Block *falseDestBlock = op.getFalseDest();
        const auto *trueDestQoalaHostBlock = moduleTranslation->getMappediQoalaBlock(trueDestBlock);
        const auto *falseDestQoalaHostBlock = moduleTranslation->getMappediQoalaBlock(falseDestBlock);
        assert(trueDestQoalaHostBlock && "Destination block not mapped!");
        assert(falseDestQoalaHostBlock && "Destination block not mapped!");

        const std::string trueTargetBlockName = trueDestQoalaHostBlock->getName();
        const std::string falseTargetBlockName = falseDestQoalaHostBlock->getName();

        iQoalaMCExpr *trueBlockSymExpr = iQoalaMCExpr::createSymbolRef(trueTargetBlockName);
        iQoalaMCExpr *falseBlockSymExpr = iQoalaMCExpr::createSymbolRef(falseTargetBlockName);
        iQoalaMCOperand *trueTargetBlockOperand = iQoalaMCOperand::createExprOperand(trueBlockSymExpr);
        iQoalaMCOperand *falseTargetBlockOperand = iQoalaMCOperand::createExprOperand(falseBlockSymExpr);

        // Process the operands of the arith.cmpi
        iQoalaRegReference *cmpOpLeft = moduleTranslation->getMappedRegRefForRoutine(cmpIOp.getLhs());
        iQoalaRegReference *cmpOpRight = moduleTranslation->getMappedRegRefForRoutine(cmpIOp.getRhs());
        iQoalaMCOperand *cmpLeftOperand = iQoalaMCOperand::createRegisterOperand(cmpOpLeft);
        iQoalaMCOperand *cmpRight1Operand = iQoalaMCOperand::createRegisterOperand(cmpOpRight);

        // Get the opcode according to the compare instruction predicate
        const QoalaHostMCInstr::OpCode opcode = getQoalaHostOpCodeFor(cmpIOp.getPredicate());
        if (opcode == QoalaHostMCInstr::OP_UNKNOWN) {
            op.emitOpError("Conditional branch: unsupported predicate");
            return failure();
        }

        // Insert the conditional branch instruction (true branch)
        const auto *condBrInstr = qoala::iqoala::helpers::buildInstruction<QoalaHostMCInstr>(
            moduleTranslation, op.getOperation(), opcode,
            {}, {}, {cmpLeftOperand, cmpRight1Operand, trueTargetBlockOperand},
            /*useOpOperands=*/false, /*appendInstruction=*/true);
        if (!condBrInstr) {
            return failure();
        }
        // Insert the unconditional jump (false branch)
        const auto *uncondBrInstr = qoala::iqoala::helpers::buildInstruction<QoalaHostMCInstr>(
            moduleTranslation, op.getOperation(), QoalaHostMCInstr::OP_JUMP,
            {}, {}, {falseTargetBlockOperand},
            /*useOpOperands=*/false, /*appendInstruction=*/true);
        if (!uncondBrInstr) {
            return failure();
        }
        return success();
    }
    // The mapped operation is not an integer comparison -> error
    op.emitOpError("Conditional Branching instruction does not make use of a comparison of integers instruction");
    return failure();
}

static LogicalResult placeNetQASMJumpInstr(ModuleTranslation *moduleTranslation, cf::BranchOp &op) {
    // For this, we assume that the first operation of the target block is the destination of the jump
    Block *destBlock = op.getDest();
    Operation *destOp = &destBlock->front();
    iQoalaMCExpr *destOpExpr = iQoalaMCExpr::createInstructionRef(destOp);
    iQoalaMCOperand *destOperand = iQoalaMCOperand::createExprOperand(destOpExpr);

    // Create the jump instruction
    const auto *instruction = qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
        moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_JMP,
        {}, {}, {destOperand},
        /*useOpOperands=*/false, /*appendInstruction=*/true);
    return instruction ? success() : failure();
}

static bool valueCanBeTracedToZeroConstant(const Value &value) {
    // We assume that we folded constants as much as possible in the previous optimization stages
    // So we can safely assume that, if the value can be traced to zero, then it *must* be yielded
    // directly by an arith.constant operation, i.e. we don't need to explore more than one operation
    // up in the data flow path.

    // If the defining op does not exist (null), then the value is *must be* a block (function) arg
    // If so, the value cannot be traced to zero
    if (Operation *definingOp = value.getDefiningOp()) {
        if (auto constantOp = dyn_cast<arith::ConstantIntOp>(definingOp)) {
            return constantOp.value() == 0;
        }
    }
    return false;
}

static SmallVector<iQoalaMCOperand *> createCmpOperands(const ModuleTranslation *moduleTranslation, arith::CmpIOp cmpIOp, bool *oneOperandIsZero) {
    // Check if one of the values used by the comparison can be traced back to zero
    SmallVector<iQoalaMCOperand *> operands;
    const bool leftIsZero = valueCanBeTracedToZeroConstant(cmpIOp.getLhs());
    const bool rightIsZero = valueCanBeTracedToZeroConstant(cmpIOp.getRhs());
    if (!leftIsZero) {
        iQoalaRegReference *cmpOpLeft = moduleTranslation->getMappedRegRefForRoutine(cmpIOp.getLhs());
        iQoalaMCOperand *cmpLeftOperand = iQoalaMCOperand::createRegisterOperand(cmpOpLeft);
        operands.push_back(cmpLeftOperand);
    }

    if (!rightIsZero) {
        iQoalaRegReference *cmpOpRight = moduleTranslation->getMappedRegRefForRoutine(cmpIOp.getRhs());
        iQoalaMCOperand *cmpRightOperand = iQoalaMCOperand::createRegisterOperand(cmpOpRight);
        operands.push_back(cmpRightOperand);
    }
    *oneOperandIsZero = leftIsZero || rightIsZero;
    return operands;
}

static LogicalResult placeNetQASMCondBrInstr(ModuleTranslation *moduleTranslation, cf::CondBranchOp &op) {
    const Value condition = op.getCondition();
    Operation *cmpOp = moduleTranslation->getMappedCmpOperation(condition);
    if (auto cmpIOp = dyn_cast<arith::CmpIOp>(cmpOp)) {
        // Process the destinations of the MLIR conditional jump: we assume it's the first operation of the block
        Operation *trueDestOp = &op.getTrueDest()->front();
        Operation *falseDestOp = &op.getFalseDest()->front();

        iQoalaMCExpr *trueBlockSymExpr = iQoalaMCExpr::createInstructionRef(trueDestOp);
        iQoalaMCExpr *falseBlockSymExpr = iQoalaMCExpr::createInstructionRef(falseDestOp);
        iQoalaMCOperand *trueTargetBlockOperand = iQoalaMCOperand::createExprOperand(trueBlockSymExpr);
        iQoalaMCOperand *falseTargetBlockOperand = iQoalaMCOperand::createExprOperand(falseBlockSymExpr);

        // Process the operands of the arith.cmpi
        bool oneOperandIsZero = false;
        SmallVector<iQoalaMCOperand *> cmpOperands = createCmpOperands(moduleTranslation, cmpIOp, &oneOperandIsZero);
        cmpOperands.push_back(trueTargetBlockOperand);

        // Get the opcode according to the compare instruction predicate
        const NetQASMMCInstr::OpCode opcode = getNetQASMOpCodeFor(cmpIOp.getPredicate(), oneOperandIsZero);
        if (opcode == NetQASMMCInstr::OP_UNKNOWN) {
            op.emitOpError("Conditional branch: unsupported predicate");
            return failure();
        }

        // Insert the conditional branch instruction (true branch)
        const auto *condBrInstr = qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
            moduleTranslation, op.getOperation(), opcode,
            {}, {}, cmpOperands,
            /*useOpOperands=*/false, /*appendInstruction=*/true);
        if (!condBrInstr) {
            return failure();
        }
        // Insert the unconditional jump (false branch)
        const auto *uncondBrInstr = qoala::iqoala::helpers::buildInstruction<NetQASMMCInstr>(
            moduleTranslation, op.getOperation(), NetQASMMCInstr::OP_JMP,
            {}, {}, {falseTargetBlockOperand},
            /*useOpOperands=*/false, /*appendInstruction=*/true);
        if (!uncondBrInstr) {
            return failure();
        }
        return success();
    }
    // The mapped operation is not an integer comparison -> error
    op.emitOpError("Conditional Branching instruction does not make use of a comparison instruction");
    return failure();
}

static LogicalResult translateControlFlowOperation(Operation *operation, ModuleTranslation *moduleTranslation) {
    LLVM_DEBUG(llvm::dbgs() << "******** Translating op '" << operation->getName() << "' *********\n");
    return llvm::TypeSwitch<Operation *, LogicalResult>(operation)
        .Case([&](cf::BranchOp op) -> LogicalResult {
            if (qoala::dialects::helpers::operationIsInsideMainFunc(operation)) {
                return placeQoalaHostJumpInstr(moduleTranslation, op);
            }
            if (qoala::dialects::helpers::operationIsInsideLocalRoutineFunc(operation)) {
                return placeNetQASMJumpInstr(moduleTranslation, op);
            }
            return op.emitOpError("Branch operation not in host or netqasm section!\n");
        })
        .Case([&](cf::CondBranchOp op) -> LogicalResult {
            if (qoala::dialects::helpers::operationIsInsideMainFunc(operation)) {
                return placeQoalaHostCondBrInstr(moduleTranslation, op);
            }
            if (qoala::dialects::helpers::operationIsInsideLocalRoutineFunc(operation)) {
                return placeNetQASMCondBrInstr(moduleTranslation, op);
            }
            return op.emitOpError("Conditional branch operation not in host or netqasm section!\n");
        })
        .Default([](Operation *op) -> LogicalResult {
            return op->emitOpError("Unknown way to translate a ControlFlow operation to iQoala: '") << *op << "'\n";
        });
}

namespace qoala::translate {
    LogicalResult ControlFlowToiQoalaTranslation::convertOperation(Operation *op, ModuleTranslation *moduleTranslation) const {
        return translateControlFlowOperation(op, moduleTranslation);
    }
}