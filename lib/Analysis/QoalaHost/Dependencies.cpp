#include <vector>
#include <unordered_set>

#include "Analysis/QoalaHost/Helpers.h"
#include "Analysis/Helpers/Helpers.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "mlir/IR/BuiltinOps.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "qoalahost-add-deps-pass-internal"


#if  __cplusplus >= 202002L
    std::string blockIDFmt = "block_{}";
#else
    std::string blockIDFmt = "block_%d";
#endif

using namespace mlir;
using namespace qoala::dialects;
using namespace qoala::analysis;

namespace qoala::analysis::dependencies {
    LogicalResult addDependencies(ModuleOp &moduleOp) {
        // This pass keeps track of dependencies betwwen the blocks, i.e. for a given block
        // which other block should be exected beforehand. At the moment, the track the
        // following dependencies:
        // - Data dependencies thourgh the dataflow graph
        // - Classical communication dependencies (to keep the ordering of the communication operations)
        // - Entanglement generation dependencies (same)
        // At the moment, it does not keep track of the predecessors at the control flow level.
        // Wether it should do it or not is still an open question, see ticket 78

        // NOTE: We make the following assumptions regarding the `return` operation:
        //
        // 1. If the `return` op returns a value, it will be captured in the data
        //    flow graph through operand dependencies. This will naturally result in
        //    block-level dependencies showing up in the graph.
        //
        // 2. If the `return` op does not return a value and exists purely for MLIR's
        //    structural correctness, then it will not introduce any explicit block
        //    dependency. In this case, it is treated as a semantic terminator with
        //    no effect on ordering.
        //
        // 3. In HIR, passes such as dead code elimination and loop unrolling are
        //    expected to run before this pass. As a result, we assume that no
        //    operations exist after the `return` op when this pass runs. Since block
        //    reordering has not yet occurred, we also assume the `return` remains at
        //    the end of its block and serves only as a semantic terminator if it
        //    returns nothing. It is the responsibility of later passes that modify
        //    block ordering to preserve this property.


        LLVM_DEBUG(llvm::dbgs() << "\n=== QoalaHostDependencies: "
                                   "Building Block Dependency Graph ===\n");

        // Block-level dependency graph: block -> set of blocks it depends on.
        // Note: we use an `std::unordered_set` rather than `llvm::SmallPtrSet` becasue we cannot estimate the size of
        // this set. Depending on the program it could grow large.
        llvm::DenseMap<Block *, std::unordered_set<Block *>> blockDeps;

        // Block -> string ID (e.g., block_0)
        llvm::DenseMap<Block *, std::string> blockIdMap;
        uint32_t idCounter = 0;

        const auto mainFuncs = moduleOp.getOps<qoalahost::MainFuncOp>();
        assert(!mainFuncs.empty() && "No main func? This is embarrassing...");
        // We get the first main func op; since it's unique in the module, it's safe to "ignore" the rest of the
        // iterator
        qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();

        for (Block &block: mainFunc.getBody().getBlocks()) {
            blockIdMap.try_emplace(&block, helpers::formatString(blockIDFmt, idCounter++));
        }

        LLVM_DEBUG(llvm::dbgs() << "\n=== Tracking all dependencies ===\n");

        std::vector<Operation *> commOps;
        std::vector<Operation *> requestCallOps;

        mainFunc.walk([&](Operation *op) {
            Block *consumerBlock = op->getBlock();

            // Track classical communication ops
            if (isa<qoalahost::SendIntsOp, qoalahost::RecvIntsOp, qoalahost::SendFloatsOp, qoalahost::RecvFloatsOp>(
                        op)) {
                commOps.push_back(op);
            }

            // Track request routine call ops
            // Note: This code checks whether the callee is a `netqasm.request_routine` by resolving
            // the symbol reference and inspecting the corresponding operation. An alternative approach
            // could be to collect all declared request routine names beforehand (e.g., in a set) and
            // then check for membership when encountering a call. However, that might introduce overhead
            // if the set grows large. For now, this direct resolution is acceptable, and the logic may
            // be refactored and put in a helper function later if needed.
            if (auto callOp = dyn_cast<qoalahost::CallOp>(op)) {
                if (const auto symRef = callOp.getCalleeAttr().dyn_cast<SymbolRefAttr>()) {
                    const Operation *callee = SymbolTable::lookupNearestSymbolFrom(moduleOp, symRef);
                    if (callee && isa<netqasm::RequestRoutineOp>(callee)) {
                        requestCallOps.push_back(op);
                    }
                }
            }

            // Track data dependencies
            for (Value operand: op->getOperands()) {
                if (Operation *producer = operand.getDefiningOp()) {
                    Block *producerBlock = producer->getBlock();
                    if (producerBlock != consumerBlock && blockDeps[consumerBlock].insert(producerBlock).second) {
                        LLVM_DEBUG(llvm::dbgs()
                                   << blockIdMap[consumerBlock] << " depends on " << blockIdMap[producerBlock] << "\n");
                    }
                }
            }
        });

        // Classical communication ordering dependencies
        LLVM_DEBUG(llvm::dbgs() << "\n=== Tracking communication dependencies ===\n");
        for (size_t i = 1; i < commOps.size(); ++i) {
            Block *prevBlock = commOps[i - 1]->getBlock();
            Block *currBlock = commOps[i]->getBlock();

            if (prevBlock != currBlock && blockDeps[currBlock].insert(prevBlock).second) {
                LLVM_DEBUG(llvm::dbgs() << blockIdMap[currBlock] << " depends on " << blockIdMap[prevBlock] << "\n");
            }
        }

        // Request routine call ordering dependencies
        LLVM_DEBUG(llvm::dbgs() << "\n=== Tracking request routines dependencies ===\n");
        for (size_t i = 1; i < requestCallOps.size(); ++i) {
            Block *prevBlock = requestCallOps[i - 1]->getBlock();
            Block *currBlock = requestCallOps[i]->getBlock();

            if (prevBlock != currBlock && blockDeps[currBlock].insert(prevBlock).second) {
                LLVM_DEBUG(llvm::dbgs() << blockIdMap[currBlock] << " depends on " << blockIdMap[prevBlock] << "\n");
            }
        }

        for (Block &block: mainFunc.getBody().getBlocks()) {
            OpBuilder builder = OpBuilder::atBlockBegin(&block);

            auto blockIdAttr = builder.getStringAttr(blockIdMap[&block]);

            // Collect and sort (for determinism) predecessor block IDs
            std::vector<StringRef> predIds;
            for (Block *pred: blockDeps[&block]) {
                predIds.emplace_back(blockIdMap[pred]);
            }
            std::sort(predIds.begin(), predIds.end());

            auto predListAttr = builder.getStrArrayAttr(predIds);

            // No existing BlkMeta, create a new one
            builder.create<qoalahost::BlkMeta>(block.front().getLoc(), blockIdAttr, predListAttr);

            LLVM_DEBUG(llvm::dbgs() << "Inserted new BlkMeta in " << blockIdMap[&block] << " with dependencies " << predListAttr
                                    << "\n");
        }

        return success();
    }
} // namespace qoala::analysis::dependencies
