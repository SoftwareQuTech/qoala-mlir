#include "mlir/IR/Operation.h"
#include "Target/iQoala/QoalaTranslationInterface.h"
#include "Target/iQoala/Dialect/Tensor/TensorToiQoalaTranslation.h"
#include "llvm/Support/Debug.h"

#include "mlir/Dialect/Tensor/IR/Tensor.h"

#define DEBUG_TYPE "tensor-translation"

using namespace mlir;

static LogicalResult translateNetQASMOperation(Operation *operation) {
    // TODO - Implement this dispatcher
    LLVM_DEBUG(llvm::dbgs() << "******** Translating op '" << operation->getName() << "' *********\n");
    return success();
}

namespace qoala::translate {
    LogicalResult TensorToiQoalaTranslation::convertOperation(Operation *op, ModuleTranslation *moduleTranslation) const {
        return translateNetQASMOperation(op);
    }
}