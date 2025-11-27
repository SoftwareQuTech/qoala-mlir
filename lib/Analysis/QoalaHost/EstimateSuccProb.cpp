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

    QoalaHostEstimateSuccProb::QoalaHostEstimateSuccProb(Operation *op, AnalysisManager &am) {

        LLVM_DEBUG(llvm::dbgs() << "Running QoalaHostQubitLifetimePass\n");

        const auto &lifetimeAnalysis = am.getAnalysis<qubitlife::QoalaHostQubitLifetime>();
        const auto lifeTimes = lifetimeAnalysis.getLifetimes();
        const auto numQubits = lifeTimes.size();

        const auto &gatecountAnalysis = am.getAnalysis<gatecount::QoalaHostGateCount>();
        const auto oneQubitGateCount = gatecountAnalysis.getDetailedOneQubitGateCount();
        const auto twoQubitGateCount = gatecountAnalysis.getDetailedTwoQubitGateCount();

        for (auto qubitId : lifeTimes) {
            float qubit_esp = 1.0;

            float exp1 = (qoalaOptQubitLifetime + qoalaOptQubitLifetime) * lifeTimes.at(qubitId.first) /
                         static_cast<float>((qoalaOptQubitLifetime * qoalaOptQubitLifetime));
            float exp2 = lifeTimes.at(qubitId.first) / static_cast<float>(qoalaOptQubitLifetime);

            qubit_esp *= (std::exp(-exp1) + std::exp(-exp2));

            qubit_esp *= pow(1 - qoalaOptSingleGateError, oneQubitGateCount.at(qubitId.first));
            qubit_esp *= pow(1 - qoalaOptTwoGateError, twoQubitGateCount.at(qubitId.first));

            LLVM_DEBUG(llvm::dbgs() << "Qubit[" << qubitId.first << "] ESP:" << qubit_esp << "\n");
            esp *= qubit_esp;
        }
        esp /= pow(2, numQubits);
        LLVM_DEBUG(llvm::dbgs() << "Totat ESP: " << esp << "\n");
    }

    bool isInvalidated(const AnalysisManager::PreservedAnalyses &pa) {
        return !pa.isPreserved<gatecount::QoalaHostGateCount>() || !pa.isPreserved<qubitlife::QoalaHostQubitLifetime>();
    }

} // namespace qoala::analysis::esp
