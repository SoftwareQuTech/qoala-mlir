#include "mlir/IR/Operation.h"
#include "Target/iQoala/QoalaTranslationInterface.h"
#include "Target/iQoala/Dialect/Arith/ArithToiQoalaTranslation.h"
#include "llvm/Support/Debug.h"

#include "mlir/Dialect/Arith/IR/Arith.h"

#define DEBUG_TYPE "arith-translation"

using namespace mlir;

static LogicalResult translateNetQASMOperation(Operation *operation) {
    // TODO - Implement this dispatcher
    LLVM_DEBUG(llvm::dbgs() << "******** Translating op '" << operation->getName() << "' *********\n");
    return success();
}

namespace qoala::translate {
    LogicalResult ArithToiQoalaTranslation::convertOperation(Operation *op, ModuleTranslation &moduleTranslation) const {
        return translateNetQASMOperation(op);
    }
}