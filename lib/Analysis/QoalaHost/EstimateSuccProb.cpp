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
using namespace qoala::dialects::qoalahost;

namespace qoala::analysis::esp {

    QoalaHostEstimateSuccProb::QoalaHostEstimateSuccProb(Operation *op, AnalysisManager &am) {

        LLVM_DEBUG(llvm::dbgs() << "Running QoalaHostQubitLifetimePass\n");

        const auto &lifetimeAnalysis = am.getAnalysis<qubitlife::QoalaHostQubitLifetime>();
        const auto lifeTimes = lifetimeAnalysis.getLifetimes();
        const auto numQubits = lifeTimes.size();

        const auto &gatecountAnalysis = am.getAnalysis<gatecount::QoalaHostGateCount>();
        const auto oneQubitGateCount = gatecountAnalysis.getDetailedOneQubitGateCount();
        const auto twoQubitGateCount = gatecountAnalysis.getDetailedTwoQubitGateCount();

        // From PhysRevLett.130.213601
        const float singleQubitOpErr = 0.01;
        const float dualQubitOpErr = 0.05;
        // micro seconds
        const uint32_t t1 = 62000;
        const uint32_t t2 = 62000;

        for (auto qubitId : lifeTimes) {
            float qubit_esp = 1.0;

            float exp1 = (t1 + t2) * lifeTimes.at(qubitId.first) / static_cast<float>((t1 * t2));
            float exp2 = lifeTimes.at(qubitId.first) / static_cast<float>(t1);

            qubit_esp *= (std::exp(-exp1) + std::exp(-exp2));

            qubit_esp *= pow(1 - singleQubitOpErr, oneQubitGateCount.at(qubitId.first));
            qubit_esp *= pow(1 - dualQubitOpErr, twoQubitGateCount.at(qubitId.first));

            LLVM_DEBUG(llvm::dbgs() << "Qubit[" << qubitId.first << "] ESP:" << qubit_esp << "\n");
            esp *= qubit_esp;
        }
        esp /= pow(2, numQubits);
        LLVM_DEBUG(llvm::dbgs() << "Totat ESP: " << esp << "\n");
        
    }

} // namespace qoala::analysis::esp
