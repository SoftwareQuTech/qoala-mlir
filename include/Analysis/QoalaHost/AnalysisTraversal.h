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
    static inline mlir::Block *scanForReadyBlock(mlir::Region &region, const llvm::DenseSet<mlir::Block *> &visited,
                                                 const BlockPrerequisites &prereqs,
                                                 const llvm::DenseSet<mlir::Block *> &condBrTargets) {

        for (mlir::Block &b : region.getBlocks()) {
            if (visited.contains(&b) || condBrTargets.contains(&b)) {
                continue;
            }

            // Check ALL dependencies are met (AND logic)
            auto depIt = prereqs.dependencies.find(&b);
            if (depIt != prereqs.dependencies.end()) {
                bool allDepsMet = true;
                for (mlir::Block *dep : depIt->second) {
                    if (!visited.contains(dep)) {
                        allDepsMet = false;
                        break;
                    }
                }
                if (!allDepsMet) {
                    continue;
                }
            }

            // Check at least ONE predecessor is met (OR logic), vacuously true if empty
            auto predIt = prereqs.predecessors.find(&b);
            if (predIt != prereqs.predecessors.end() && !predIt->second.empty()) {
                bool anyPredMet = false;
                for (mlir::Block *pred : predIt->second) {
                    if (visited.contains(pred)) {
                        anyPredMet = true;
                        break;
                    }
                }
                if (!anyPredMet) {
                    continue;
                }
            }

            return &b;
        }
        return nullptr;
    }

    /**
     * Build BlockPrerequisites from blk_meta attributes on all blocks in mainFunc.
     * Extracts predecessors, dependencies, prev_ent and prev_comm.
     */
    static inline BlockPrerequisites buildBlockPrerequisites(dialects::qoalahost::MainFuncOp &mainFunc) {
        llvm::StringMap<mlir::Block *> idToBlock;
        for (auto &block : mainFunc.getBlocks()) {
            if (auto blkMeta = mlir::dyn_cast<dialects::qoalahost::BlkMeta>(block.front())) {
                idToBlock[blkMeta.getBlockId()] = &block;
            }
        }

        BlockPrerequisites prereqs;

        for (auto &block : mainFunc.getBlocks()) {
            auto blkMeta = mlir::dyn_cast<dialects::qoalahost::BlkMeta>(block.front());
            if (!blkMeta) {
                continue;
            }
            if (const mlir::ArrayAttr a = blkMeta.getPredecessorsAttr()) {
                for (const mlir::StringRef s : a.getAsValueRange<mlir::StringAttr>()) {
                    auto it = idToBlock.find(s);
                    if (it != idToBlock.end()) {
                        prereqs.predecessors[&block].push_back(it->second);
                    }
                }
            }
            if (const mlir::ArrayAttr a = blkMeta.getDependenciesAttr()) {
                for (const mlir::StringRef s : a.getAsValueRange<mlir::StringAttr>()) {
                    auto it = idToBlock.find(s);
                    if (it != idToBlock.end()) {
                        prereqs.dependencies[&block].push_back(it->second);
                    }
                }
            }
            if (const mlir::StringAttr a = blkMeta.getPrevEntAttr()) {
                if (!a.getValue().empty()) {
                    auto it = idToBlock.find(a.getValue());
                    if (it != idToBlock.end()) {
                        prereqs.dependencies[&block].push_back(it->second);
                    }
                }
            }
            if (const mlir::StringAttr a = blkMeta.getPrevCommAttr()) {
                if (!a.getValue().empty()) {
                    auto it = idToBlock.find(a.getValue());
                    if (it != idToBlock.end()) {
                        prereqs.dependencies[&block].push_back(it->second);
                    }
                }
            }
        }

        return prereqs;
    }

} // namespace qoala::analysis

#endif // QOALAHOST_BLOCK_TRAVERSAL_H
