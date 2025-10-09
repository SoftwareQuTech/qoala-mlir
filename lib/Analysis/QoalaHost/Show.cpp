#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/QoalaHost/Passes.h"

#define DEBUG_TYPE "qoalahost-show-analysis"

using namespace mlir;

namespace qoala::analysis {
#define GEN_PASS_DEF_QOALAHOSTSHOWANALYSIS
#include "Dialect/QoalaHost/Passes.h.inc"

   class QoalaHostShowAnalysis
            : public impl::QoalaHostShowAnalysisBase<QoalaHostShowAnalysis> {

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
            llvm::outs() << "  [Qubits Lifetimes]: \n";
            const auto &analysis = getAnalysis<qbitlife::QoalaHostQubitLifeTime>();
            const auto lifeTimes = analysis.getLifeTimes();
            for (auto const& [qubit, life] : lifeTimes) {
                llvm::outs() << "    - " << qubit << ": " << life << "\n";
            }
        }
    }
} // namespace qoala::analysis
