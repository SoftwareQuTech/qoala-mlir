#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/QoalaHost/Passes.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "qoalahost-show-analysis-pass"

using namespace mlir;

namespace qoala::analysis {
#define GEN_PASS_DEF_QOALAHOSTSHOWANALYSIS
#include "Dialect/QoalaHost/Passes.h.inc"

    class QoalaHostShowAnalysisPass
        : public impl::QoalaHostShowAnalysisBase<QoalaHostShowAnalysisPass> {
        using QoalaHostShowAnalysisBase::QoalaHostShowAnalysisBase;
        void runOnOperation() override;
    };

    void QoalaHostShowAnalysisPass::runOnOperation() {
        auto &analysis = getAnalysis<QoalaHostQMemoryEfficiency>();
        llvm::outs() << "Efficiency = " << analysis.getEfficiency() << "\n";

    }
} // namespace qoala::analysis
