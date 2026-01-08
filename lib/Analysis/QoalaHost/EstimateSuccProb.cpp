#include "Analysis/NetQASM/Helpers.h"
#include "Analysis/QoalaHost/Helpers.h"
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

namespace qoala::analysis::esp {

    static float calculateQubitEsp(const float lifetime, const uint32_t oneQubitGates, const uint32_t twoQubitGates) {
        // qubitEsp = prod(gate_fidelities) * [exp(-(T1+T2)/(T1*T2) * lifetime)+exp(-lifetime/T1)]
        // where fidelity = (1 - error_rate)^gate_counts
        float qubitEsp = 1.0f;
        // decay factors
        float t1t2tDecayExp = (qoalaOptQubitLifetime + qoalaOptQubitLifetime) * lifetime /
                              static_cast<float>((qoalaOptQubitLifetime * qoalaOptQubitLifetime));
        float t1DecayExp = lifetime / static_cast<float>(qoalaOptQubitLifetime);

        LLVM_DEBUG(llvm::dbgs() << "e^-" << t1t2tDecayExp << " + e^-" << t1DecayExp << " = " << std::exp(-t1t2tDecayExp)
                                << " + " << std::exp(-t1DecayExp) << " \n");

        qubitEsp *= (std::exp(-t1t2tDecayExp) + std::exp(-t1DecayExp));

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
        const auto numQubits = lifeTimes.size();

        // Get the gate count
        const auto &gatecountAnalysis = am.getAnalysis<gatecount::QoalaHostGateCount>();
        const auto oneQubitGateCount = gatecountAnalysis.getDetailedOneQubitGateCount();
        const auto twoQubitGateCount = gatecountAnalysis.getDetailedTwoQubitGateCount();

        // For each qubit compute the ESP as:
        // qubitEsp = prod(gate_fidelities) * exp(-T1/T2 * lifetime) where fidelity = (1 - error_rate)^gate_counts
        // Total ESP is given by the product of each qubit ESP

        for (auto qubitId : lifeTimes) {
            auto lifetime = lifeTimes.at(qubitId.first);
            auto oneQubitGates = oneQubitGateCount.at(qubitId.first);
            auto twoQubitGates = twoQubitGateCount.at(qubitId.first);

            auto qubitEsp = calculateQubitEsp(lifetime, oneQubitGates, twoQubitGates);
            LLVM_DEBUG(llvm::dbgs() << "Qubit[" << qubitId.first << "] ESP:" << qubitEsp << "\n");

            totalEsp *= qubitEsp;
        }
        // This is a coefficient obtained by collecting the 1/2 from each qubit fidelity formula
        totalEsp /= pow(2, numQubits);
        LLVM_DEBUG(llvm::dbgs() << "Totat ESP: " << totalEsp << "\n");
    }

    // This analysis is preserved as long as its dependencies are preserved
    bool isInvalidated(const AnalysisManager::PreservedAnalyses &pa) {
        return !pa.isPreserved<gatecount::QoalaHostGateCount>() || !pa.isPreserved<qubitlife::QoalaHostQubitLifetime>();
    }

} // namespace qoala::analysis::esp
