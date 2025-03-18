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

static LogicalResult placeUnconditionalBranchingInstr(ModuleTranslation *moduleTranslation, cf::BranchOp &op) {
    // Get the name of the target block
    const Block *destBlock = op.getDest();
    const auto *destQoalaHostBlock = moduleTranslation->getMappediQoalaBlock(destBlock);
    assert(destQoalaHostBlock && "Destination block not mapped!");
    const std::string targetBlockName = destQoalaHostBlock->getName();

    // Use the target name as an operand
    SmallVector<iQoalaMCOperand *> processedOperands;
    iQoalaMCExpr *blockSymExpr = iQoalaMCExpr::createSymbolRef(targetBlockName);
    iQoalaMCOperand *targetBlockOperand = iQoalaMCOperand::createExprOperand(blockSymExpr);
    processedOperands.push_back(targetBlockOperand);

    // Create the jump instruction
    const auto *instruction = qoala::iqoala::helpers::buildInstruction<QoalaHostMCInstr>(
        moduleTranslation, op.getOperation(), QoalaHostMCInstr::OP_JUMP,
        std::nullopt, std::nullopt, processedOperands,
        /*useOpOperands=*/false, /*appendInstruction=*/true);
    return instruction ? success() : failure();
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

static LogicalResult placeCondBranchingInstr(ModuleTranslation *moduleTranslation, cf::CondBranchOp &op) {
    // In MLIR, conditional branch instructions are semantically different (assuming eq comparison):
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
    // beq(%op1, %op2) : <dest_if_true>
    // jump() : <dest_if_false>

    const Value condition = op.getCondition();
    Operation *cmpOp = moduleTranslation->getMappedCmpOperation(condition);
    if (auto cmpIOp = dyn_cast<arith::CmpIOp>(cmpOp)) {
        // Process the destinations of the MLIR conditional jump
        const Block *trueDestBlock = op.getTrueDest();
        const Block *falseDestBlock = op.getTrueDest();
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
        iQoalaRegReference *cmpOpLeft = moduleTranslation->getMappedRegReference(cmpIOp.getLhs());
        iQoalaRegReference *cmpOpRight = moduleTranslation->getMappedRegReference(cmpIOp.getRhs());
        iQoalaMCOperand *cmpLeftOperand = iQoalaMCOperand::createRegisterOperand(cmpOpLeft);
        iQoalaMCOperand *cmpRight1Operand = iQoalaMCOperand::createRegisterOperand(cmpOpRight);

        // Get the opcode according to the compare instruction predicate
        const QoalaHostMCInstr::OpCode opcode = getQoalaHostOpCodeFor(cmpIOp.getPredicate());
        if (opcode == QoalaHostMCInstr::OP_UNKNOWN) {
            op.emitError("Conditional branch: unsupported predicate");
            return failure();
        }

        // Insert the conditional branch instruction (true branch)
        const auto *condBrInstr = qoala::iqoala::helpers::buildInstruction<QoalaHostMCInstr>(
            moduleTranslation, op.getOperation(), opcode,
            std::nullopt, std::nullopt, {cmpLeftOperand, cmpRight1Operand, trueTargetBlockOperand},
            /*useOpOperands=*/false, /*appendInstruction=*/true);
        // Insert the unconditional jump (false branch)
        const auto *uncondBrInstr = qoala::iqoala::helpers::buildInstruction<QoalaHostMCInstr>(
            moduleTranslation, op.getOperation(), QoalaHostMCInstr::OP_JUMP,
            std::nullopt, std::nullopt, {falseTargetBlockOperand},
            /*useOpOperands=*/false, /*appendInstruction=*/true);
        return condBrInstr || uncondBrInstr ? success() : failure();
    }
    // The mapped operation is not an integer comparison -> error
    op.emitError("Conditional Branching instruction does not make use of a comparison instruction");
    return failure();
}

static LogicalResult translateControlFlowOperation(Operation *operation, ModuleTranslation *moduleTranslation) {
    LLVM_DEBUG(llvm::dbgs() << "******** Translating op '" << operation->getName() << "' *********\n");
    return llvm::TypeSwitch<Operation *, LogicalResult>(operation)
        .Case([&](cf::BranchOp op) -> LogicalResult {
            if (qoala::dialects::helpers::operationIsInsideMainFunc(operation)) {
                return placeUnconditionalBranchingInstr(moduleTranslation, op);
            }
            if (qoala::dialects::helpers::operationIsInsideLocalRoutineFunc(operation)) {
                // TODO
                return success();
            }
            return op.emitError("Branch operation not in host or netqasm section!") << *op << "\n";
        })
        .Case([&](cf::CondBranchOp op) -> LogicalResult {
            if (qoala::dialects::helpers::operationIsInsideMainFunc(operation)) {
                return placeCondBranchingInstr(moduleTranslation, op);
            }
            if (qoala::dialects::helpers::operationIsInsideLocalRoutineFunc(operation)) {
                // TODO
                return success();
            }
            return op.emitError("Conditional branch operation not in host or netqasm section!") << *op << "\n";
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