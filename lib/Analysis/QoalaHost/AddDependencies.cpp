#include <vector>
#include <unordered_set>

#include "Dialect/QoalaHost/Passes.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "qoalahost-add-deps-pass"

using namespace mlir;
using namespace qoala::dialects::qoalahost;

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

        // Walk all operations and build the block dependency graph
        operation->walk([&](Operation *op) {
            Block *consumerBlock = op->getBlock();

            for (Value operand : op->getOperands()) {
                if (Operation *producer = operand.getDefiningOp()) {
                    Block *producerBlock = producer->getBlock();

                    if (producerBlock != consumerBlock) {
                        // Insert only if not already present (avoid duplicate prints)
                        if (blockDeps[consumerBlock].insert(producerBlock).second) {
                            LLVM_DEBUG(llvm::dbgs() << "Block \n"); 
                            consumerBlock->print(llvm::dbgs());
                            LLVM_DEBUG(llvm::dbgs() << "depends on block:\n");
                            producerBlock->print(llvm::dbgs());
                            LLVM_DEBUG(llvm::dbgs() << "\n");
                        }
                    }
                }
            }
        });
    }
} /* namespace qoala::analysis */
