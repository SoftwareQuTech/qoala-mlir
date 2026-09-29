#include "Analysis/Helpers/Helpers.h"

#include "mlir/IR/BuiltinOps.h"

using namespace mlir;

namespace qoala::helpers {
    std::optional<Operation *> getNextOperation(Operation *op) {
        Block::iterator it(op);
        ++it;
        if (it != op->getBlock()->end()) {
            return &*it;
        }
        return std::nullopt;
    }
} // namespace qoala::helpers
