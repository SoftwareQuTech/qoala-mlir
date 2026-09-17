#include "Analysis/QoalaHost/AnalysisTraversal.h"

using namespace mlir;

namespace qoala::analysis {
    Block *scanForReadyBlock(Region &region, const llvm::DenseSet<Block *> &visited, const BlockPrerequisites &prereqs,
                             const llvm::DenseSet<Block *> &condBrTargets) {

        for (Block &b : region.getBlocks()) {
            if (visited.contains(&b) || condBrTargets.contains(&b)) {
                continue;
            }

            // Check ALL dependencies are met (AND logic)
            if (prereqs.dependencies.contains(&b)) {
                bool allDepsMet = true;
                for (const Block *dep : prereqs.dependencies.at(&b)) {
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
                for (const Block *pred : prereqs.predecessors.at(&b)) {
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
        llvm::StringMap<Block *> idToBlock;
        for (auto &block : mainFunc.getBlocks()) {
            if (auto blkMeta = dyn_cast<dialects::qoalahost::BlkMeta>(block.front())) {
                idToBlock[blkMeta.getBlockId()] = &block;
            }
        }

        BlockPrerequisites prereqs;

        for (auto &block : mainFunc.getBlocks()) {
            auto blkMeta = dyn_cast<dialects::qoalahost::BlkMeta>(block.front());
            if (!blkMeta) {
                continue;
            }
            if (const ArrayAttr a = blkMeta.getPredecessorsAttr()) {
                for (const StringRef s : a.getAsValueRange<StringAttr>()) {
                    if (idToBlock.contains(s)) {
                        prereqs.predecessors[&block].push_back(idToBlock.at(s));
                    }
                }
            }
            if (const ArrayAttr a = blkMeta.getDependenciesAttr()) {
                for (const StringRef s : a.getAsValueRange<StringAttr>()) {
                    if (idToBlock.contains(s)) {
                        prereqs.dependencies[&block].push_back(idToBlock.at(s));
                    }
                }
            }
            if (const StringAttr a = blkMeta.getPrevEntAttr()) {
                if (!a.getValue().empty()) {
                    if (idToBlock.contains(a.getValue())) {
                        prereqs.dependencies[&block].push_back(idToBlock.at(a.getValue()));
                    }
                }
            }
            if (const StringAttr a = blkMeta.getPrevCommAttr()) {
                if (!a.getValue().empty()) {
                    if (idToBlock.contains(a.getValue())) {
                        prereqs.dependencies[&block].push_back(idToBlock.at(a.getValue()));
                    }
                }
            }
        }

        return prereqs;
    }

} // namespace qoala::analysis
