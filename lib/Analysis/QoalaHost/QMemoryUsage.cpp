#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/QoalaHost/Passes.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "llvm/Support/Debug.h"
#include "Dialect/QoalaHost/QoalaHost.h"

#define DEBUG_TYPE "qoalahost-qmemory-usage"

using namespace mlir;
using namespace qoala::dialects;

namespace qoala::analysis {
#define GEN_PASS_DEF_QOALAHOSTQMEMORYUSAGE
#include "Dialect/QoalaHost/Passes.h.inc"

    class QoalaHostQMemoryUsagePass
        : public impl::QoalaHostQMemoryUsageBase<QoalaHostQMemoryUsagePass> {
        using QoalaHostQMemoryUsageBase::QoalaHostQMemoryUsageBase;
        
        mlir::Pass::Statistic logicalQubit{this, "logic-qubit", "Number of logical qubits declared in the program."};
        mlir::Pass::Statistic physicalQubit{this, "phys-qubit", "Number of physical qubit that are going to be used to execute the program."};

        void runOnOperation() override;
    };

    void QoalaHostQMemoryUsagePass::runOnOperation() {
        LLVM_DEBUG(llvm::dbgs() << "Running QoalaHostQMemoryUsagePass\n");

        Operation *operation = getOperation();
        for (Region &reg: operation->getRegions()) {
            for (Block &block: reg.getBlocks()) {
                for (Operation &func: block.getOperations()) {
                    if (auto mainFunc = dyn_cast<qoalahost::MainFuncOp>(&func)) {
                        // LLVM_DEBUG(llvm::dbgs() << mainFunc.getSymName() << "\n");
                        ++logicalQubit;
                        ++physicalQubit;
                        llvm::outs() << mainFunc.getSymName() << "\n";
                    }
                }
            }
        }
        // Do the same thing with a walk operation
    }
} // namespace qoala::analysis
