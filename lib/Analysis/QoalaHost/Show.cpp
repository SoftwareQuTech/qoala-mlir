#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/QoalaHost/Passes.h"

#define DEBUG_TYPE "qoalahost-show-analysis"

using namespace mlir;

namespace qoala::analysis {
#define GEN_PASS_DEF_QOALAHOSTSHOWANALYSIS
#include "Dialect/QoalaHost/Passes.h.inc"

    class QoalaHostShowAnalysis : public impl::QoalaHostShowAnalysisBase<QoalaHostShowAnalysis> {

    public:
        using QoalaHostShowAnalysisBase::QoalaHostShowAnalysisBase;

        void runOnOperation() override;
    };

    void QoalaHostShowAnalysis::runOnOperation() {
        llvm::outs() << "--- QoalaHost Show Analysis Pass ---\n";

        if (showQMemEff) {
            const auto &analysis = getAnalysis<qmemeff::QoalaHostQMemoryEfficiency>();
            llvm::outs() << "  [QMem Efficiency]:" << analysis.getEfficiency() << "\n";
        }

        if (showQubitLife) {
            const auto &analysis = getAnalysis<qbitlife::QoalaHostQubitLifeTime>();
            llvm::outs() << "  [Qubits Lifetimes]: \n";
            const auto lifeTimes = analysis.getLifeTimes();
            for (const auto &entry : lifeTimes) {
                llvm::outs() << "    - " << entry.first << ": " << entry.second << "\n";
            }
        }
    }
} // namespace qoala::analysis
