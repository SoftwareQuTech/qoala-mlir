#ifndef QOALAHOST_BLOCK_TRAVERSAL_H
#define QOALAHOST_BLOCK_TRAVERSAL_H

#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "mlir/IR/Block.h"
#include "mlir/IR/Region.h"

#include "Dialect/QoalaHost/QoalaHost.h"

namespace qoala::analysis {

    /**
     * Block readiness prerequisites, split by semantics:
     * - dependencies (from dependencies, prev_comm, prev_ent): ALL must be visited
     * - predecessors (from predecessors): at least ONE must be visited
     * - cf.br bypasses the above prerequisites
     */
    struct BlockPrerequisites {
        mlir::DenseMap<mlir::Block *, llvm::SmallVector<mlir::Block *, 2>> dependencies;
        mlir::DenseMap<mlir::Block *, llvm::SmallVector<mlir::Block *, 2>> predecessors;
    };

    /**
     * Scan all blocks in physical order and return the first unvisited block
     * whose blk_meta prerequisites are met, skipping forbidden condBrTargets.
     * Physical order is the tie-breaker for equally-ready blocks.
     */
    mlir::Block *scanForReadyBlock(mlir::Region &region, const llvm::DenseSet<mlir::Block *> &visited,
                                   const BlockPrerequisites &prereqs,
                                   const llvm::DenseSet<mlir::Block *> &condBrTargets);

    /**
     * Build BlockPrerequisites from blk_meta attributes on all blocks in mainFunc.
     * Extracts predecessors, dependencies, prev_ent and prev_comm.
     */
    BlockPrerequisites buildBlockPrerequisites(dialects::qoalahost::MainFuncOp &mainFunc);

} // namespace qoala::analysis

#endif // QOALAHOST_BLOCK_TRAVERSAL_H
