#ifndef QOALA_MLIR_FOLDING_H
#define QOALA_MLIR_FOLDING_H

#include "mlir/IR/Operation.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Transforms/FoldUtils.h"

#include <vector>

namespace qoala::helpers::folding {
    /**
     * Small class that keeps track of all the constants that are marked for removal.
     * It implements methods from RewriterBase::Listener to keep track of the folder instructions
     * and instructions that can be safely removed after folding.
     */
    class FolderTracker : public mlir::RewriterBase::Listener {
        // All constants in the operation post folding.
        std::vector<mlir::Operation *> existingConstants;

    public:
        std::vector<mlir::Operation *> getExistingConstants() { return existingConstants; }

        void notifyOperationInserted(mlir::Operation *op) override { existingConstants.push_back(op); }

        void notifyOperationRemoved(mlir::Operation *op) override {
            if (const auto it = llvm::find(existingConstants, op); it != existingConstants.end()) {
                existingConstants.erase(it);
            }
        }
    };

    /**
     * Simple analysis method that folds instructions statically if it is possible.
     * This is used for folding constants (arith.constants declared statically, operated,
     * and then used within the same scope) and remove them to simplify any further analysis.
     * @param op The operation to analyze for constant folding
     * @return Weather the analysis succeeded or failed
     */
    template<typename OpTy>
    mlir::LogicalResult foldConstants(OpTy &op) {
        std::vector<mlir::Operation *> ops;
        FolderTracker folderTracker;
        mlir::OperationFolder folderHelper(op.getContext(), /*listener=*/&folderTracker);

        // We just walk over all the instructions, discovering them for potential folding
        op.template walk<mlir::WalkOrder::PreOrder>([&](mlir::Operation *operation) { ops.push_back(operation); });

        // Visit the discovered ops in reverse order, so we don't break data dependencies
        for (mlir::Operation *operation : llvm::reverse(ops)) {
            (void) folderHelper.tryToFold(operation);
        }

        // Finally, remove all orphaned constants after folding them
        for (const auto cst : folderTracker.getExistingConstants()) {
            if (cst->use_empty()) {
                cst->erase();
            }
        }
        return mlir::success();
    }
} // namespace qoala::helpers::folding

#endif // QOALA_MLIR_FOLDING_H
