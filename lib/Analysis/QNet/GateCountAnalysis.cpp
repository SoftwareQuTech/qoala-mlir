#include "Analysis/QNet/Helpers.h"
#include "Dialect/QNet/Passes.h"
#include "Dialect/QNet/QNet.h"

#define DEBUG_TYPE "qnet-show-analysis-pass-gate-count"

using namespace mlir;

namespace qoala::analysis {
#define GEN_PASS_DEF_QNETGATECOUNTSHOWANALYSIS
#include "Dialect/QNet/Passes.h.inc"

    class QNetGateCountShowAnalysis
        : public impl::QNetGateCountShowAnalysisBase<QNetGateCountShowAnalysis> {
        using QNetGateCountShowAnalysisBase::QNetGateCountShowAnalysisBase;

        void runOnOperation() override;
    };

    void QNetGateCountShowAnalysis::runOnOperation() {
        const auto &analysis = getAnalysis<gatecount::QNetGateCount>();
        llvm::outs() << "  [Gate Count]:\n";
        llvm::outs() << "  - One-qubit gates: "<< analysis.getOneQubitGateCount() << "\n";
        llvm::outs() << "  - Two-qubit gates: "<< analysis.getTwoQubitGateCount() << "\n";
        llvm::outs() << "  - Total gates: "<< analysis.getGateCount() << "\n";

        llvm::outs() << "\n  Per qubit gate count:\n";
        llvm::outs() << "  - One-qubit gates:\n";
        for (auto &pair : analysis.getOneQubitGateCounts()) {
            llvm::outs() << "    * qubit[" << pair.getKey() << "]: " << pair.second << "\n";
        }
        
        llvm::outs() << "  - Two-qubit gates:\n";
        for (auto &pair : analysis.getTwoQubitGateCounts()) {
            llvm::outs() << "    * qubit[" << pair.getKey() << "]: " << pair.second << "\n";
        }

        llvm::outs() << "  -  All gates:\n";
        for (auto &pair : analysis.getGateCounts()) {
            llvm::outs() << "    * qubit[" << pair.getKey() << "]: " << pair.second << "\n";
        }

    }
} // namespace qoala::analysis