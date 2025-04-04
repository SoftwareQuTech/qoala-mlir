#include <vector>
#include <unordered_set>

#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "qoalahost-add-deps-pass-internal"

using namespace mlir;
using namespace qoala::dialects;
using namespace qoala::analysis;

namespace qoala::analysis::dependencies {
    LogicalResult addDependencies(ModuleOp &moduleOp) {
        // NOTE: We make the following assumptions regarding the `return` operation:
        //
        // 1. If the `return` op returns a value, it will be captured in the data
        // flow
        //    graph through operand dependencies. This will naturally result in
        //    block-level dependencies showing up in the graph.
        //
        // 2. If the `return` op does not return a value and exists purely for
        // MLIR's
        //    structural correctness, then it will not introduce any explicit block
        //    dependency. In this case, it is treated as a semantic terminator with
        //    no effect on ordering.
        //
        // 3. In HIR passes, we expect  transformations such as dead code
        // elimination and loop
        //    unrolling to ensure that the `return` op remains correctly placed at
        //    the end of a region or block. This pass does not enforce reordering
        //    constraints for return; it is the responsibility of those later
        //    passes.
        //
        // 4. In some edge cases, a data dependency may exist between the `return`
        // op and another
        //    operation that is internal to the same block. In such cases, we assume
        //    that other passes will lift the producer operation into a separate
        //    block as needed, allowing the dependency to be reflected in the block
        //    dependency graph properly.

        LLVM_DEBUG(llvm::dbgs() << "\n=== QoalaHostDependencies: "
                                   "Building Block Dependency Graph ===\n");

        // Block-level dependency graph: block -> set of blocks it depends on.
        // Note: we use an `std::unordered_set` rather than `llvm::SmallPtrSet` becasue we cannot estimate the size of
        // this set. Depending on the program it could grow large.
        llvm::DenseMap<Block *, std::unordered_set<Block *>> blockDeps;

        // Block -> string ID (e.g., block_0)
        llvm::DenseMap<Block *, std::string> blockIdMap;
        int idCounter = 0;

        mlir::ModuleOp mod = llvm::cast<mlir::ModuleOp>(moduleOp);
        auto mainFuncs = mod.getOps<qoalahost::MainFuncOp>();
        assert(!mainFuncs.empty() && "No main func? This is embarrassing...");
        // We get the first main func op; since it's unique in the module, it's safe to "ignore" the rest of the
        // iterator
        qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();

        for (Block &block: mainFunc.getBody().getBlocks()) {
            std::string id = "block_" + std::to_string(idCounter++);
            blockIdMap[&block] = id;
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
                auto symRef = callOp.getCalleeAttr().dyn_cast<SymbolRefAttr>();
                if (symRef) {
                    Operation *callee = SymbolTable::lookupNearestSymbolFrom(moduleOp, symRef);
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

            std::string blockId = blockIdMap[&block];
            auto blockIdAttr = builder.getStringAttr(blockId);

            // Collect and sort (for determinism) predecessor block IDs
            std::vector<mlir::StringRef> predIds;
            for (Block *pred: blockDeps[&block]) {
                predIds.push_back(blockIdMap[pred]);
            }
            std::sort(predIds.begin(), predIds.end());

            auto predListAttr = builder.getStrArrayAttr(predIds);

            builder.create<qoalahost::BlkMeta>(block.front().getLoc(), blockIdAttr, predListAttr);

            LLVM_DEBUG(llvm::dbgs() << "Inserted BlkMeta in " << blockId << " with dependencies " << predListAttr
                                    << "\n");
        }

        return success();
    }
} // namespace qoala::analysis::dependencies
