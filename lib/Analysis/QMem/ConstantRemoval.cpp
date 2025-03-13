#include "mlir/IR/BuiltinOps.h"
#include "llvm/ADT/TinyPtrVector.h"
#include "mlir/Transforms/FoldUtils.h"
#include "Dialect/QMem/QMem.h"

#include "Analysis/Helpers/Helpers.h"

#include "llvm/Support/Debug.h"

using namespace mlir;
using namespace qoala::helpers;

#define DEBUG_TYPE "const-removal"

namespace qoala::helpers {
    /**
     * Small class that keeps that of all the constants that are marked for removal
     * It implements methods from RewriterBase::Listener to keep track of the folder instructions
     * and instructions that can be safely removed after folding.
     */
    class FolderTracker : public RewriterBase::Listener {
        // All constants in the operation post folding.
        std::vector<Operation *> existingConstants;
    public:
        std::vector<Operation *> getExistingConstants() { return existingConstants; }
        void notifyOperationInserted(Operation *op) override {
            existingConstants.push_back(op);
        }
        void notifyOperationRemoved(Operation *op) override {
            if (const auto it = llvm::find(existingConstants, op); it != existingConstants.end())
                existingConstants.erase(it);
        }
    };

    LogicalResult foldConstants(ModuleOp &module) {
        std::vector<Operation *> ops;
        FolderTracker folderTracker;
        OperationFolder folderHelper(module.getContext(), /*listener=*/&folderTracker);

        // We just walk over all the instructions, discovering them for potential folding
        module->walk<WalkOrder::PreOrder>([&](Operation *op) { ops.push_back(op); });
        LLVM_DEBUG(llvm::dbgs() << "Number discovered ops: " << ops.size() << "\n");

        // Visit the discovered ops in reverse order, so we don't break data dependencies
        std::ranges::reverse(ops.begin(), ops.end());
        for (Operation *op : ops) {
            LLVM_DEBUG(llvm::dbgs() << "Trying to fold op: " << *op << "\n");
            (void)folderHelper.tryToFold(op);
        }

        // Finally, remove all orphaned constants after folding them
        for (auto cst : folderTracker.getExistingConstants()) {
            LLVM_DEBUG(llvm::dbgs() << "Trying to delete constant " << *cst << "\n");
            if (cst->use_empty()) {
                cst->erase();
            }
        }
        return success();
    }
} /* namespace qoala::analysis */