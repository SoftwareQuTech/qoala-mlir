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

static LogicalResult placeUncondBranchingInstr(ModuleTranslation *moduleTranslation, cf::BranchOp &op) {
    const Block *destBlock = op.getDest();
    const auto *destQoalaHostBlock = moduleTranslation->getMappediQoalaBlock(destBlock);
    assert(destQoalaHostBlock && "Destination block not mapped!");
    const std::string targetBlockName = destQoalaHostBlock->getName();

    SmallVector<iQoalaMCOperand *> processedOperands;
    iQoalaMCExpr *blockSymExpr = iQoalaMCExpr::createSymbolRef(targetBlockName);
    iQoalaMCOperand *targetBlockOperand = iQoalaMCOperand::createExprOperand(blockSymExpr);
    processedOperands.push_back(targetBlockOperand);

    const auto *instruction = qoala::iqoala::helpers::buildInstruction<QoalaHostMCInstr>(
        moduleTranslation, op.getOperation(), QoalaHostMCInstr::OP_JUMP,
        std::nullopt, std::nullopt, processedOperands,
        /*useOpOperands=*/false, /*appendInstruction=*/true);
    return instruction ? success() : failure();
}

static LogicalResult placeCondBranchingInstr(const ModuleTranslation *moduleTranslation, cf::CondBranchOp &op) {
    const Value condition = op.getCondition();
    Operation *cmpOp = moduleTranslation->getMappedCmpOperation(condition);
    if (auto cmpIOp = dyn_cast<arith::CmpIOp>(cmpOp)) {
        // TODO
        return success();
    }
    op.emitError("Conditional Branching instruction does not make use of a comparison instruction");
    return failure();
}

static LogicalResult translateControlFlowOperation(Operation *operation, ModuleTranslation *moduleTranslation) {
    LLVM_DEBUG(llvm::dbgs() << "******** Translating op '" << operation->getName() << "' *********\n");
    return llvm::TypeSwitch<Operation *, LogicalResult>(operation)
        .Case([&](cf::BranchOp op) -> LogicalResult {
            if (qoala::dialects::helpers::operationIsInsideMainFunc(operation)) {
                return placeUncondBranchingInstr(moduleTranslation, op);
            }
            if (qoala::dialects::helpers::operationIsInsideLocalRoutineFunc(operation)) {
                // TODO
                return success();
            }
            return op.emitError("Branch operation not in host or netqasm section!") << *op << "\n";
        })
        .Case([&](cf::CondBranchOp op) -> LogicalResult {
            if (qoala::dialects::helpers::operationIsInsideMainFunc(operation)) {
                // TODO
                return success();
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