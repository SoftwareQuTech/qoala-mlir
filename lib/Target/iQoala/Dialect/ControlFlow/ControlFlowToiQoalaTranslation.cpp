#include "mlir/IR/Operation.h"
#include "llvm/ADT/TypeSwitch.h"
#include "Target/iQoala/Dialect/ControlFlow/ControlFlowToiQoalaTranslation.h"

#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "builtin-translation"

using namespace mlir;
using namespace qoala::translate;

static LogicalResult translateControlFlowOperation(Operation *operation, ModuleTranslation *moduleTranslation) {
    LLVM_DEBUG(llvm::dbgs() << "******** Translating op '" << operation->getName() << "' *********\n");
    return llvm::TypeSwitch<mlir::Operation *, LogicalResult>(operation)
        .Case([&](cf::BranchOp op) -> LogicalResult {
            return success();
        })
        .Case([&](cf::CondBranchOp op) -> LogicalResult {
            return success();
        })
        .Default([](Operation *op) -> LogicalResult {
            return op->emitOpError("Unknown way to translate a ControlFlow operation to iQoala: '") << *op << "'\n";
        });
    return success();
}

namespace qoala::translate {
    LogicalResult ControlFlowToiQoalaTranslation::convertOperation(Operation *op, ModuleTranslation *moduleTranslation) const {
        return translateControlFlowOperation(op, moduleTranslation);
    }
}