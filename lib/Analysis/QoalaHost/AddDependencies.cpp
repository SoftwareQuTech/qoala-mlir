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

    class QoalaHostAddBlockDependenciesPass : public impl::QoalaHostAddBlockDependenciesBase<QoalaHostAddBlockDependenciesPass> {
        using QoalaHostAddBlockDependenciesBase::QoalaHostAddBlockDependenciesBase;
        void runOnOperation() override;
    };


    void QoalaHostAddBlockDependenciesPass::runOnOperation() {
        // LLVM_DEBUG(llvm::dbgs() << "Hello world!\n");
        // TODO: try this https://mlir.llvm.org/docs/Tutorials/UnderstandingTheIRStructure/ :check:
        // TODO: try to do this with a walker :check:
        // Operation *operation = getOperation();

        // ModuleOp module = llvm::dyn_cast<ModuleOp>(operation);
        // for (Region &reg : operation->getRegions()) {
        //     for (Block &blk : reg.getBlocks()) {
        //         for (Operation &func : blk.getOperations()) {
        //             if(auto mainFunc = dyn_cast<MainFuncOp>(&func)) {
        //                 LLVM_DEBUG(llvm::dbgs() << mainFunc.getSymName() << "\n");
        //             }
        //         }
        //     }
        // }
        // operation->walk([&](MainFuncOp mainFunc) {
        //     LLVM_DEBUG(llvm::dbgs() << mainFunc.getSymName() << "\n");
        // });
        // operation->walk([&](Block *block) {
        //     LLVM_DEBUG(llvm::dbgs() << "Block: " << block << "\n");
    
        //     for (auto *pred : block->getPredecessors()) {
        //         LLVM_DEBUG(llvm::dbgs() << "  Predecessor: " << pred << "\n");
        //     }
        // });
        LLVM_DEBUG(llvm::dbgs() << "\n=== QoalaHostAddBlockDependenciesPass: Building Dependency Graph ===\n");

        Operation *operation = getOperation();

        // The dependency graph: from consumer -> list of producers
        llvm::DenseMap<Operation *, llvm::SmallVector<Operation *, 4>> dependencyGraph;

        // All operations we encounter (nodes)
        llvm::SmallVector<Operation *, 32> allOps;

        // Walk all ops and collect data dependencies
        operation->walk([&](Operation *op) {
            allOps.push_back(op);

            for (Value operand : op->getOperands()) {
                if (Operation *producer = operand.getDefiningOp()) {
                    dependencyGraph[op].push_back(producer);
                    LLVM_DEBUG(llvm::dbgs() << "Dependency: " << *op << " ← " << *producer << "\n");
                }
            }
        });
    }
} /* namespace qoala::analysis */
