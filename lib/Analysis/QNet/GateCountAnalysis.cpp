#include "Analysis/QNet/Helpers.h"
#include "Dialect/QNet/Passes.h"
#include "Dialect/QNet/QNet.h"

#define DEBUG_TYPE "qnet-show-analysis-pass-gate-count"

using namespace mlir;

namespace qoala::analysis {
#define GEN_PASS_DEF_QNETGATECOUNTSHOWANALYSIS
#include "Dialect/QNet/Passes.h.inc"

    class QNetGateCountShowAnalysis : public impl::QNetGateCountShowAnalysisBase<QNetGateCountShowAnalysis> {
        using QNetGateCountShowAnalysisBase::QNetGateCountShowAnalysisBase;

        void runOnOperation() override;
    };

    void QNetGateCountShowAnalysis::runOnOperation() {
        const auto &analysis = getAnalysis<gatecount::QNetGateCount>();
        const auto &oneQubitGateCounts = analysis.getOneQubitGateCounts();
        const auto &twoQubitGateCounts = analysis.getTwoQubitGateCounts();
        const auto &gateCounts = analysis.getGateCounts();

        std::vector<llvm::StringRef> sortedQubits;
        sortedQubits.reserve(oneQubitGateCounts.size());
        for (const auto &item : oneQubitGateCounts) {
            sortedQubits.push_back(item.getKey());
        }
        std::sort(sortedQubits.begin(), sortedQubits.end());

        llvm::outs() << "  [Gate Count]:\n";
        llvm::outs() << "  - One-qubit gates: " << analysis.getOneQubitGateCount() << "\n";
        llvm::outs() << "  - Two-qubit gates: " << analysis.getTwoQubitGateCount() << "\n";
        llvm::outs() << "  - Total gates: " << analysis.getGateCount() << "\n";

        llvm::outs() << "\n  Per qubit gate count:\n";
        llvm::outs() << "  - One-qubit gates:\n";

        for (const auto &qubit : sortedQubits) {
            llvm::outs() << "    * qubit[" << qubit << "]: " << oneQubitGateCounts.at(qubit) << "\n";
        }

        llvm::outs() << "  - Two-qubit gates:\n";
        for (const auto &qubit : sortedQubits) {
            llvm::outs() << "    * qubit[" << qubit << "]: " << twoQubitGateCounts.at(qubit) << "\n";
        }

        llvm::outs() << "  -  All gates:\n";
        for (const auto &qubit : sortedQubits) {
            llvm::outs() << "    * qubit[" << qubit << "]: " << gateCounts.at(qubit) << "\n";
        }
    }
} // namespace qoala::analysis
