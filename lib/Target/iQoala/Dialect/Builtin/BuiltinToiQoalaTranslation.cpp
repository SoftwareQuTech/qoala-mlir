#include "mlir/IR/Operation.h"
#include "Target/iQoala/QoalaTranslationInterface.h"
#include "Target/iQoala/Dialect/Builtin/BuiltinToiQoalaTranslation.h"
#include "llvm/Support/Debug.h"

#include "mlir/IR/BuiltinDialect.h"

#define DEBUG_TYPE "builtin-translation"

using namespace mlir;

static LogicalResult translateBuiltinOperation(Operation *operation) {
    // TODO - Implement this dispatcher
    LLVM_DEBUG(llvm::dbgs() << "******** Translating op '" << operation->getName() << "' *********\n");
    return success();
}

namespace qoala::translate {
    LogicalResult BuiltinToiQoalaTranslation::convertOperation(Operation *op, ModuleTranslation &moduleTranslation) const {
        return translateBuiltinOperation(op);
    }
}