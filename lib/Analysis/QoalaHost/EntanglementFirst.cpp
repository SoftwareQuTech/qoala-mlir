#include "Analysis/Helpers/Helpers.h"
#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "llvm/Support/Debug.h"
#include "mlir/IR/BuiltinOps.h"

using namespace mlir;
using namespace qoala::dialects;
using namespace qoala::analysis;

#define DEBUG_TYPE "qoalahost-entanglement-first"

namespace qoala::analysis::reordering {
    void groupEntanglementBlocksFirst(ModuleOp moduleOp) {

        const auto mainFuncs = moduleOp.getOps<qoalahost::MainFuncOp>();
        assert(!mainFuncs.empty() && "No main func? This is embarrassing...");

        qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();

        const llvm::StringMap<Operation *> routineMap = collectRoutineMap(moduleOp);

        SmallVector<Block *> entBlocks;
        SmallVector<Block *> otherBlocks;

        for (auto &blk : mainFunc) {
            Operation *firstOp = &*blk.begin();
            if (auto iface = llvm::dyn_cast<helpers::QuantumOpInterface>(firstOp)) {
                if (iface.getBlockType(routineMap) == BlockType::QC) {
                    entBlocks.push_back(&blk);
                    continue;
                }
            }

            otherBlocks.push_back(&blk);
        }

        // Move entanglement blocks to the beginning of the region
        Block *insertBefore = &mainFunc.getBlocks().front();
        for (Block *blk : entBlocks) {
            if (blk != insertBefore) {
                blk->moveBefore(insertBefore);
            }
            insertBefore = blk->getNextNode();
        }

        // Leave all other blocks as-is (or optionally sort them later)
        LLVM_DEBUG(llvm::dbgs() << "[Grouping] Moved " << entBlocks.size()
                                << " entanglement blocks to the beginning\n");
    }

    void moveRemoteReferencesBlockToBegin(ModuleOp &moduleOp) {
        const auto mainFuncs = moduleOp.getOps<qoalahost::MainFuncOp>();
        assert(!mainFuncs.empty() && "No main func? This is embarrassing.");
        qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();

        const auto frontInsertionPoint = &mainFunc.front();
        for (Block &block : mainFunc.getBlocks()) {
            const bool blockHasRemoteRefPlaceholder =
                    std::any_of(block.getOperations().begin(), block.getOperations().end(),
                                [](const Operation &op) { return isa<qoalahost::RemoteIDRefOp>(op); });
            if (blockHasRemoteRefPlaceholder) {
                block.moveBefore(frontInsertionPoint);
            }
        }
    }
} // namespace qoala::analysis::reordering
