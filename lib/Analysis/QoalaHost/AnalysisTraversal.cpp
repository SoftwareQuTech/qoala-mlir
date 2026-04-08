#include "Analysis/QoalaHost/AnalysisTraversal.h"

namespace qoala::analysis {

    mlir::Block *scanForReadyBlock(mlir::Region &region, const llvm::DenseSet<mlir::Block *> &visited,
                                   const BlockPrerequisites &prereqs,
                                   const llvm::DenseSet<mlir::Block *> &condBrTargets) {

        for (mlir::Block &b : region.getBlocks()) {
            if (visited.contains(&b) || condBrTargets.contains(&b)) {
                continue;
            }

            // Check ALL dependencies are met (AND logic)
            if (prereqs.dependencies.contains(&b)) {
                bool allDepsMet = true;
                for (const mlir::Block *dep : prereqs.dependencies.at(&b)) {
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
            if (prereqs.predecessors.contains(&b) && !prereqs.predecessors.at(&b).empty()) {
                bool anyPredMet = false;
                for (mlir::Block *pred : prereqs.predecessors.at(&b)) {
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

    BlockPrerequisites buildBlockPrerequisites(dialects::qoalahost::MainFuncOp &mainFunc) {
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
                    if (idToBlock.contains(s)) {
                        prereqs.predecessors[&block].push_back(idToBlock.at(s));
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
