#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/QoalaHost/Passes.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/Statistic.h"
#include "Dialect/QoalaHost/QoalaHost.h"

#define DEBUG_TYPE "qoalahost-qmemory-usage"

using namespace mlir;
using namespace qoala::dialects;

namespace qoala::analysis {

#define GEN_PASS_DEF_QOALAHOSTQMEMORYUSAGE
#include "Dialect/QoalaHost/Passes.h.inc"

    // Global statistics
    // Declare global statistic counters outside the class to avoid issues with
    // deleted copy constructors in llvm::Statistic (due to std::atomic members).
    // This also sidesteps the need to explicitly clone statistic fields in the pass.
    STATISTIC(logicalQubit, "Number of logical qubits declared in the program.");
    STATISTIC(physicalQubit, "Number of physical qubits used by the program.");

    class QoalaHostQMemoryUsagePass 
        : public impl::QoalaHostQMemoryUsageBase<QoalaHostQMemoryUsagePass> {
        using QoalaHostQMemoryUsageBase::QoalaHostQMemoryUsageBase;
        
        void runOnOperation() override;
    };

    void QoalaHostQMemoryUsagePass::runOnOperation() {
        LLVM_DEBUG(llvm::dbgs() << "Running QoalaHostQMemoryUsagePass\n");

        Operation *operation = getOperation();
        for (Region &reg: operation->getRegions()) {
            for (Block &block: reg.getBlocks()) {
                for (Operation &func: block.getOperations()) {
                    if (auto mainFunc = dyn_cast<qoalahost::MainFuncOp>(&func)) {
                        // Logic to count qubits would go here
                        ++logicalQubit;
                        ++physicalQubit;
                        llvm::outs() << "Found main func: " << mainFunc.getSymName() << "\n";
                    }
                }
            }
        }
    }
} // namespace qoala::analysis
