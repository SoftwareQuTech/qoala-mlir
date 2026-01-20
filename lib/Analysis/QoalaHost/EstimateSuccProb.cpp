#include "Analysis/QoalaHost/GateCount.h"
#include "Analysis/QoalaHost/Helpers.h"
#include "Analysis/QoalaHost/QubitLife.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "llvm/ADT/TypeSwitch.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "mlir/IR/Operation.h"

#define DEBUG_TYPE "qoalahost-esp"

using namespace mlir;
using namespace qoala::analysis;
using namespace qoala::dialects::qoalahost;
using namespace qoala::options;

namespace qoala::analysis::fidelity {

    static float calculateQubitEsp(const float lifetime, const uint32_t oneQubitGates, const uint32_t twoQubitGates) {
        // qubitEsp = prod(gate_fidelities) *  1/2*(exp(-lifetime/T2) + 1)
        // where fidelity = (1 - error_rate)^gate_counts
        float qubitEsp = 1.0f;
        // decoherence factor
        float t2Exp = lifetime / static_cast<float>(qoalaOptQubitLifetime);

        float qubitDecoherence = (std::exp(-t2Exp) + 1) / 2;

        LLVM_DEBUG(llvm::dbgs() << "0.5*e^-" << t2Exp << " + 0.5" << " = " << qubitDecoherence << " \n");

        qubitEsp *= qubitDecoherence;

        qubitEsp *= std::pow(1.0f - qoalaOptSingleGateError, oneQubitGates);
        qubitEsp *= std::pow(1.0f - qoalaOptDualGateError, twoQubitGates);

        return qubitEsp;
    }

    QoalaHostEstimateSuccProb::QoalaHostEstimateSuccProb(Operation *op, AnalysisManager &am) {

        // Compute the Estimated Succes Probability (ESP) of a programm.
        // Use qubit lifetime and gate count for the computation.
        // Qubits IDs found with lifetime analysis are assumed to map, one to one,
        // to the ones fuound with gate count analysis. This shoudl hold true as the IDs are
        // derived from block and operations index, and any pass that would change such properties should invalidate
        // lifetime, requiring this pass to recompute lifetime right before gatecount.
        LLVM_DEBUG(llvm::dbgs() << "Running QoalaHostESPPass\n");

        // Get the qubit lifetime
        const auto &lifetimeAnalysis = am.getAnalysis<qubitlife::QoalaHostQubitLifetime>();
        const auto lifeTimes = lifetimeAnalysis.getLifetimes();

        // Get the gate count
        const auto &gatecountAnalysis = am.getAnalysis<gatecount::QoalaHostGateCount>();
        const auto oneQubitGateCount = gatecountAnalysis.getDetailedOneQubitGateCount();
        const auto twoQubitGateCount = gatecountAnalysis.getDetailedTwoQubitGateCount();

        // For each qubit compute the ESP as:
        // qubitEsp = prod(gate_fidelities) * 1/2*(exp(-lifetime/T2) + 1) where fidelity = (1 - error_rate)^gate_counts
        // Total ESP is given by the product of each qubit ESP

        for (auto qubitId : lifeTimes) {
            auto lifetime = lifeTimes.at(qubitId.first);
            auto oneQubitGates = oneQubitGateCount.at(qubitId.first);
            auto twoQubitGates = twoQubitGateCount.at(qubitId.first);

            auto qubitEsp = calculateQubitEsp(lifetime, oneQubitGates, twoQubitGates);
            LLVM_DEBUG(llvm::dbgs() << "Qubit[" << qubitId.first << "] ESP:" << qubitEsp << "\n");

            totalEsp *= qubitEsp;
        }

        LLVM_DEBUG(llvm::dbgs() << "Totat ESP: " << totalEsp << "\n");
    }

    // This analysis is preserved as long as its dependencies are preserved
    bool isInvalidated(const AnalysisManager::PreservedAnalyses &pa) {
        return !pa.isPreserved<gatecount::QoalaHostGateCount>() || !pa.isPreserved<qubitlife::QoalaHostQubitLifetime>();
    }

} // namespace qoala::analysis::fidelity
