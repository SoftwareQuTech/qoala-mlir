#include "Analysis/QoalaHost/QubitLife.h"
#include "Dialect/QoalaHost/Passes.h"
#include "Dialect/QoalaHost/QoalaHost.h"

#define DEBUG_TYPE "qoalahost-show-analysis-pass-qubit-life"

using namespace mlir;

namespace qoala::analysis {
#define GEN_PASS_DEF_QOALAHOSTQUBITLIFESHOWANALYSIS
#include "Dialect/QoalaHost/Passes.h.inc"

    class QoalaHostQubitLifeShowAnalysis
        : public impl::QoalaHostQubitLifeShowAnalysisBase<QoalaHostQubitLifeShowAnalysis> {
        using QoalaHostQubitLifeShowAnalysisBase::QoalaHostQubitLifeShowAnalysisBase;

        void runOnOperation() override;
    };

    void QoalaHostQubitLifeShowAnalysis::runOnOperation() {
        const auto &analysis = getAnalysis<qubitlife::QoalaHostQubitLifetime>();
        llvm::outs() << "  [Qubits Lifetimes]: \n";
        const auto lifeTimes = analysis.getLifetimes();
        std::vector<std::string> sortedKeys;
        sortedKeys.reserve(lifeTimes.size());
        for (const auto &[qubitID, lifetime] : lifeTimes) {
            sortedKeys.push_back(qubitID);
        }
        std::sort(sortedKeys.begin(), sortedKeys.end());

        for (const auto &key : sortedKeys) {
            llvm::outs() << "    - " << key << ": " << lifeTimes.at(key) << "\n";
        }
    }
} // namespace qoala::analysis
