#include "Analysis/QoalaHost/GateCount.h"
#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/QoalaHost/Passes.h"
#include "Dialect/QoalaHost/QoalaHost.h"

#define DEBUG_TYPE "qoalahost-show-analysis-pass-gate-count"

using namespace mlir;

namespace qoala::analysis {
#define GEN_PASS_DEF_QOALAHOSTGATECOUNTSHOWANALYSIS
#include "Dialect/QoalaHost/Passes.h.inc"

    class QoalaHostGateCountShowAnalysis
        : public impl::QoalaHostGateCountShowAnalysisBase<QoalaHostGateCountShowAnalysis> {
        using QoalaHostGateCountShowAnalysisBase::QoalaHostGateCountShowAnalysisBase;

        void runOnOperation() override;
    };

    void QoalaHostGateCountShowAnalysis::runOnOperation() {
        const auto &analysis = getAnalysis<gatecount::QoalaHostGateCount>();
        const auto &detailedOneQubitGateCount = analysis.getDetailedOneQubitGateCount();
        const auto &detailedTwoQubitGateCount = analysis.getDetailedTwoQubitGateCount();
        const auto &detailedGateCount = analysis.getDetailedGateCount();

        std::vector<std::string> sortedQubits;
        sortedQubits.reserve(detailedOneQubitGateCount.size());
        for (const auto &item : detailedOneQubitGateCount) {
            sortedQubits.push_back(item.first);
        }
        std::sort(sortedQubits.begin(), sortedQubits.end());

        llvm::outs() << "  [Gate Count]:\n";
        llvm::outs() << "  - One-qubit gates: " << analysis.getOneQubitGateCount() << "\n";
        llvm::outs() << "  - Two-qubit gates: " << analysis.getTwoQubitGateCount() << "\n";
        llvm::outs() << "  - Total gates: " << analysis.getGateCount() << "\n";

        llvm::outs() << "\n  Detailed gate count:\n";
        llvm::outs() << "  - One-qubit gates:\n";

        for (const auto &qubit : sortedQubits) {
            llvm::outs() << "    * qubit[" << qubit << "]: " << detailedOneQubitGateCount.at(qubit) << "\n";
        }

        llvm::outs() << "  - Two-qubit gates:\n";
        for (const auto &qubit : sortedQubits) {
            llvm::outs() << "    * qubit[" << qubit << "]: " << detailedTwoQubitGateCount.at(qubit) << "\n";
        }

        llvm::outs() << "  -  All gates:\n";
        for (const auto &qubit : sortedQubits) {
            llvm::outs() << "    * qubit[" << qubit << "]: " << detailedGateCount.at(qubit) << "\n";
        }
    }
} // namespace qoala::analysis
