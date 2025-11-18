#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "llvm/ADT/TypeSwitch.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "mlir/IR/Operation.h"

#define DEBUG_TYPE "qoalahost-esp"

using namespace mlir;
using namespace qoala::dialects;

namespace qoala::analysis::esp {

    QoalaHostEstimateSuccProb::QoalaHostEstimateSuccProb(Operation *op, AnalysisManager &am) {
        // Bla bla
        auto module = dyn_cast<ModuleOp>(*op);

        const auto &analysis = am.getAnalysis<qubitlife::QoalaHostQubitLifetime>();
        const auto lifeTimes = analysis.getLifetimes();
        const auto numQubits = lifeTimes.size();

        auto mainFuncs = module.getOps<qoalahost::MainFuncOp>();

        assert(!mainFuncs.empty() && "No main function found in module.");

        qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();

        const auto callOps = mainFunc.getOps<qoalahost::CallOp>();

        const float singleQubitOpErr = 0.01;
        const float dualQubitOpErr = 0.1;
        const uint32_t t1 = 500;
        const uint32_t t2 = 500;

        for (auto callOp : callOps) {
            auto callee = callOp.getCalleeOperation<FunctionOpInterface>();
            callee.walk([&](Operation *opInCallee) {
                llvm::TypeSwitch<Operation *>(opInCallee)
                        .Case([&](const netqasm::ifaces::MeasOp measureOp) {
                            LLVM_DEBUG(llvm::dbgs() << "Found MeasureOp: " << measureOp << "\n");
                            gate_esp *= (1 - singleQubitOpErr);
                        })
                        .Case<netqasm::ifaces::SingleQubitOp>([&](const netqasm::ifaces::SingleQubitOp singleQubitOp) {
                            LLVM_DEBUG(llvm::dbgs() << "Found SingleQubitOp: " << singleQubitOp << "\n");
                            gate_esp *= (1 - singleQubitOpErr);
                        })
                        .Case<netqasm::ifaces::DualQubitOp>([&](const netqasm::ifaces::DualQubitOp dualQubitOp) {
                            LLVM_DEBUG(llvm::dbgs() << "Found DualQubitOp: " << dualQubitOp << "\n");
                            gate_esp *= (1 - dualQubitOpErr);
                        });
            });
        }

        llvm::outs() << "Gate ESP: " << gate_esp << ".\n";

        for (const auto &pair : lifeTimes) {

            float exp1 = (t1 + t2) * pair.second / static_cast<float>((t1 * t2));
            float exp2 = pair.second / static_cast<float>(t1);

            qubit_esp *= (std::exp(-exp1) + std::exp(-exp2));
        }
        qubit_esp /= pow(2, numQubits);
        llvm::outs() << "Qubit ESP: " << qubit_esp << ".\n";

        total_esp *= (gate_esp * qubit_esp);
    }

    float QoalaHostEstimateSuccProb::getESP() const { return total_esp; }

} // namespace qoala::analysis::esp
