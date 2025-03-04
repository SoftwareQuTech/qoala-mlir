#include "mlir/IR/BuiltinOps.h"
#include "llvm/ADT/TinyPtrVector.h"
#include "Dialect/QMem/QMem.h"

#include "Analysis/Helpers/Helpers.h"

#include "llvm/Support/Debug.h"

using namespace mlir;
using namespace qoala::helpers;

#define DEBUG_TYPE "const-removal"

namespace qoala::helpers {

    static bool canBeRemoved(const arith::ConstantOp &op) {
        if (op->getUsers().empty()) {
            // Base case: the constant has no uses
            return true;
        }
        return false;
    }

    LogicalResult removeOrphanConstants(ModuleOp &module) {
        LLVM_DEBUG(llvm::dbgs() << *module << "\n");
        auto mainFunctions = module.getOps<dialects::qmem::FuncOp>();
        assert(!mainFunctions.empty() && "main function is empty");

        dialects::qmem::FuncOp mainFunction = *mainFunctions.begin();
        // We need to  first mark the operations to delete, so we don't mess the "getOps" iterator
        std::vector<Operation *> toDelete;

        for (arith::ConstantOp op : mainFunction.getOps<arith::ConstantOp>()) {
            LLVM_DEBUG(llvm::dbgs() << op << "\n");
            if (canBeRemoved(op)) {
                LLVM_DEBUG(llvm::dbgs() << "Marking to delete: " << *op << "\n");
                toDelete.push_back(op.getOperation());
            }
        }

        // Perform the deletions
        for (auto *constToDelete : toDelete) {
            LLVM_DEBUG(llvm::dbgs() << "Deleting: " << *constToDelete << "\n");
            constToDelete->erase();
        }
        return success();
    }
} /* namespace qoala::analysis */