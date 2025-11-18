#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/QoalaHost/Passes.h"
#include "Dialect/QoalaHost/QoalaHost.h"

#define DEBUG_TYPE "qoalahost-show-analysis-pass-esp"

using namespace mlir;

namespace qoala::analysis {
#define GEN_PASS_DEF_QOALAHOSTESPSHOWANALYSIS
#include "Dialect/QoalaHost/Passes.h.inc"

    class QoalaHostESPShowAnalysis : public impl::QoalaHostESPShowAnalysisBase<QoalaHostESPShowAnalysis> {
        using QoalaHostESPShowAnalysisBase::QoalaHostESPShowAnalysisBase;

        void runOnOperation() override;
    };

    void QoalaHostESPShowAnalysis::runOnOperation() {
        const auto &analysis = getAnalysis<esp::QoalaHostEstimateSuccProb>();
        llvm::outs() << "  [ESP]: " << analysis.getESP() << "\n";
    }
} // namespace qoala::analysis
