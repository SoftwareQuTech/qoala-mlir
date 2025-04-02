#include <vector>
#include <unordered_set>

#include "Dialect/QoalaHost/Passes.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "qoalahost-add-deps-pass"

using namespace mlir;
using namespace qoala::dialects;

namespace qoala::analysis {
#define GEN_PASS_DEF_QOALAHOSTADDBLOCKDEPENDENCIES
#include "Dialect/QoalaHost/Passes.h.inc"

class QoalaHostAddBlockDependenciesPass
    : public impl::QoalaHostAddBlockDependenciesBase<
          QoalaHostAddBlockDependenciesPass> {
    using QoalaHostAddBlockDependenciesBase::QoalaHostAddBlockDependenciesBase;
    void runOnOperation() override;
};

void QoalaHostAddBlockDependenciesPass::runOnOperation() {
    LLVM_DEBUG(llvm::dbgs() << "\n=== QoalaHostAddBlockDependenciesPass: "
                               "Building Block Dependency Graph ===\n");

    Operation *operation = getOperation();

    // Block-level dependency graph: block -> set of blocks it depends on.
    // We use a set to avoid duplicated dependencies between blocks.
    llvm::DenseMap<Block *, std::unordered_set<Block *>> blockDeps;

    // Block -> string ID (e.g., block_0)
    llvm::DenseMap<Block *, std::string> blockIdMap;
    int idCounter = 0;

    // Assign a unique ID to each block inside qoalahost.main_func
    operation->walk([&](qoalahost::MainFuncOp mainFunc) {
        for (Block &block : mainFunc.getBody().getBlocks()) {
            std::string id = "block_" + std::to_string(idCounter++);
            blockIdMap[&block] = id;
        }

        // Track classical communication operations order
        LLVM_DEBUG(llvm::dbgs()
                   << "\n=== Tracking communication dependencies ===\n");
        std::vector<Operation *> commOps;
        mainFunc.walk([&](Operation *op) {
            if (isa<qoalahost::SendIntsOp, qoalahost::RecvIntsOp,
                    qoalahost::SendFloatsOp, qoalahost::RecvFloatsOp>(op)) {
                commOps.push_back(op);
            }
        });

        // Add dependencies based on order of classical communication ops
        for (size_t i = 1; i < commOps.size(); ++i) {
            Block *prevBlock = commOps[i - 1]->getBlock();
            Block *currBlock = commOps[i]->getBlock();

            if (prevBlock != currBlock) {
                // Insert dependency: currBlock depends on prevBlock
                if (blockDeps[currBlock].insert(prevBlock).second) {
                    LLVM_DEBUG(llvm::dbgs() << blockIdMap[currBlock] << " \n");
                    currBlock->print(llvm::dbgs());
                    LLVM_DEBUG(llvm::dbgs() << "classical comm depends on "
                                            << blockIdMap[prevBlock] << ":\n");
                    prevBlock->print(llvm::dbgs());
                    LLVM_DEBUG(llvm::dbgs() << "\n");
                }
            }
        }

        // Track call to request routine operations order
        LLVM_DEBUG(llvm::dbgs()
                   << "\n=== Tracking request routines dependencies ===\n");
        std::vector<Operation *> requestCallOps;
        mainFunc.walk([&](Operation *op) {
            if (auto callOp = dyn_cast<qoalahost::CallOp>(op)) {
                auto symRef = callOp.getCalleeAttr().dyn_cast<SymbolRefAttr>();
                if (!symRef)
                    return;

                Operation *callee =
                    SymbolTable::lookupNearestSymbolFrom(operation, symRef);
                if (callee && isa<netqasm::RequestRoutineOp>(callee)) {
                    requestCallOps.push_back(op);
                }
            }
        });

        // Add dependencies based on order of calls to request routines
        for (size_t i = 1; i < requestCallOps.size(); ++i) {
            Block *prevBlock = requestCallOps[i - 1]->getBlock();
            Block *currBlock = requestCallOps[i]->getBlock();

            if (prevBlock != currBlock) {
                // Insert dependency: currBlock depends on prevBlock
                if (blockDeps[currBlock].insert(prevBlock).second) {
                    LLVM_DEBUG(llvm::dbgs() << blockIdMap[currBlock] << " \n");
                    currBlock->print(llvm::dbgs());
                    LLVM_DEBUG(llvm::dbgs() << "quantum comm depends on "
                                            << blockIdMap[prevBlock] << ":\n");
                    prevBlock->print(llvm::dbgs());
                    LLVM_DEBUG(llvm::dbgs() << "\n");
                }
            }
        }

        // Walk all operations in the main_func to build dependency graph
        LLVM_DEBUG(llvm::dbgs() << "\n=== Tracking data dependencies ===\n");
        mainFunc.walk([&](Operation *op) {
            Block *consumerBlock = op->getBlock();

            for (Value operand : op->getOperands()) {
                if (Operation *producer = operand.getDefiningOp()) {
                    Block *producerBlock = producer->getBlock();

                    // Insert only if not already present (avoid duplicate
                    // prints)
                    if (producerBlock != consumerBlock) {
                        if (blockDeps[consumerBlock]
                                .insert(producerBlock)
                                .second) {
                            LLVM_DEBUG(llvm::dbgs()
                                       << blockIdMap[consumerBlock] << " \n");
                            consumerBlock->print(llvm::dbgs());
                            LLVM_DEBUG(llvm::dbgs()
                                       << "depends on "
                                       << blockIdMap[producerBlock] << ":\n");
                            producerBlock->print(llvm::dbgs());
                            LLVM_DEBUG(llvm::dbgs() << "\n");
                        }
                    }
                }
            }
        });

        // Insert NopMetaOp into each block with its ID and predecessor IDs
        OpBuilder builder(mainFunc.getContext());

        for (Block &block : mainFunc.getBody().getBlocks()) {
            builder.setInsertionPointToStart(&block);

            std::string blockId = blockIdMap[&block];
            auto blockIdAttr = builder.getStringAttr(blockId);

            // Collect and sort (for determinism) predecessor block IDs
            std::vector<std::string> predIds;
            for (Block *pred : blockDeps[&block]) {
                predIds.push_back(blockIdMap[pred]);
            }
            std::sort(predIds.begin(), predIds.end());

            std::vector<Attribute> predIdAttrs;
            for (const auto &id : predIds) {
                predIdAttrs.push_back(builder.getStringAttr(id));
            }

            auto predListAttr = builder.getArrayAttr(predIdAttrs);

            builder.create<qoalahost::NopMetaOp>(block.front().getLoc(),
                                                 blockIdAttr, predListAttr);

            LLVM_DEBUG(llvm::dbgs()
                       << "Inserted NopMetaOp in " << blockId
                       << " with dependencies " << predListAttr << "\n");
        }
    });
}
} // namespace qoala::analysis
