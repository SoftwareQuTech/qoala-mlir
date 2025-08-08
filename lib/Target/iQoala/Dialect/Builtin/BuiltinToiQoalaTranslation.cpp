#include "Target/iQoala/Dialect/Builtin/BuiltinToiQoalaTranslation.h"
#include "mlir/IR/Operation.h"

#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "builtin-translation"

using namespace mlir;

static LogicalResult translateBuiltinOperation(Operation *operation) {
    LLVM_DEBUG(llvm::dbgs() << "******** Translating builtin op '" << operation->getName() << "' *********\n");
    // Nothing to do here - the only operation from builtin we expect is module, which should not be translated
    if (isa<UnrealizedConversionCastOp>(operation)) {
        operation->emitOpError("Unexpected unrealized cast - Please check the type conversion in the "
                               "lowering logic - How did such an operation end up in LIR?");
    }
    assert(isa<mlir::ModuleOp>(operation) && "Builtin operation not a module!");
    return success();
}

namespace qoala::translate {
    LogicalResult BuiltinToiQoalaTranslation::convertOperation(Operation *op, ModuleTranslation *) const {
        return translateBuiltinOperation(op);
    }
} // namespace qoala::translate
