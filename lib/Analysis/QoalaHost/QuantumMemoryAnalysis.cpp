#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/QoalaHost/Passes.h"
#include "Dialect/QoalaHost/QoalaHost.h"

#define DEBUG_TYPE "qoalahost-show-analysis-pass-qmem-eff"

using namespace mlir;

namespace qoala::analysis {
#define GEN_PASS_DEF_QOALAHOSTQMEMEFFSHOWANALYSIS
#include "Dialect/QoalaHost/Passes.h.inc"

    class QoalaHostQMemEffShowAnalysis : public impl::QoalaHostQMemEffShowAnalysisBase<QoalaHostQMemEffShowAnalysis> {
        using QoalaHostQMemEffShowAnalysisBase::QoalaHostQMemEffShowAnalysisBase;

        void runOnOperation() override;
    };

    void QoalaHostQMemEffShowAnalysis::runOnOperation() {
        const auto &analysis = getAnalysis<qmemeff::QoalaHostQMemoryEfficiency>();
        llvm::outs() << "Efficiency = " << analysis.getEfficiency() << "\n";
    }
} // namespace qoala::analysis
