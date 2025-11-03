#include "Analysis/QNet/Helpers.h"
#include "Dialect/QNet/Passes.h"
#include "Dialect/QNet/QNet.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/Debug.h"
#include "mlir/IR/AsmState.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"

#define DEBUG_TYPE "qnet-gate-count"

using namespace mlir;
using namespace qoala::dialects::qnet;

namespace qoala::analysis::gatecount {

    bool isQuantumOp(mlir::Operation *op) {
        return llvm::isa<NewQubitOp, RotXOp, RotYOp, RotZOp, HadamardOp, CnotOp, CzOp, CrotXOp, MeasureOp, EprsOp,
                         EprsMeasureOp>(op);
    }

    bool isOneQubitOp(mlir::Operation *op) {
        return llvm::isa<NewQubitOp, RotXOp, RotYOp, RotZOp, HadamardOp, MeasureOp, EprsOp, EprsMeasureOp>(op);
    }

    bool isTwoQubitOp(mlir::Operation *op) { return llvm::isa<CnotOp, CzOp, CrotXOp>(op); }

    std::string getSSAName(Value val, AsmState &state) {
        std::string ssaName;
        llvm::raw_string_ostream stream(ssaName);
        val.printAsOperand(stream, state);

        return ssaName;
    }

    QNetGateCount::QNetGateCount(Operation *op) {
        llvm::StringMap<std::vector<Operation *>> qubitsToOps;
        llvm::DenseMap<Value, std::string> qubitsInitToOpRes;

        if (ModuleOp module = llvm::dyn_cast<ModuleOp>(op)) {

            AsmState state(module);

            // Walk over all ops in the module.
            module.walk([&](Operation *op) {
                if (llvm::isa<NewQubitOp, EprsOp>(op)) {
                    LLVM_DEBUG(llvm::dbgs() << "New Qubit Op for qubit: " << op->getName().getStringRef() << ".\n");
                    for (Value result : op->getResults()) {
                        std::string qubitSsaName = getSSAName(result, state);
                        qubitsInitToOpRes[result] = qubitSsaName;
                        qubitsToOps.try_emplace(qubitSsaName);
                        qubitsToOps[qubitSsaName].push_back(op);
                        qubitToOneGateCount[qubitSsaName] = 0;
                        qubitToTwoGateCount[qubitSsaName] = 0;
                        qubitToGateCount[qubitSsaName] = 0;
                    }
                } else {
                    if (isQuantumOp(op)) {
                        ++gateCount;
                        if (isTwoQubitOp(op)) {
                            LLVM_DEBUG(llvm::dbgs() << "Two Qubit Op: " << op->getName().getStringRef() << ".\n");
                            ++twoQubitGateCount;
                        } else {
                            LLVM_DEBUG(llvm::dbgs() << "One Qubit Op: " << op->getName().getStringRef() << ".\n");
                            ++oneQubitGateCount;
                        }
                        // For each operand, find its init SSA name and propagate it to the
                        // corresponding result.
                        for (auto operandIt : llvm::enumerate(op->getOperands())) {
                            Value operand = operandIt.value();
                            unsigned index = operandIt.index();

                            // Check if the operand is a qubit we are tracking
                            auto initSSAIt = qubitsInitToOpRes.find(operand);
                            if (initSSAIt != qubitsInitToOpRes.end()) {
                                std::string initSSA = initSSAIt->second;

                                // Add this operation to the history of the qubit it acts on.
                                // To avoid duplicates for multi-qubit gates, we can check first.
                                // if (qubitsToOps[initSSA].empty() || qubitsToOps[initSSA].back() != op) {
                                qubitsToOps[initSSA].push_back(op);
                                qubitToGateCount[initSSA]++;
                                if (isTwoQubitOp(op)) {
                                    qubitToTwoGateCount[initSSA]++;
                                } else {
                                    qubitToOneGateCount[initSSA]++;
                                }
                                // }

                                // Propagate the ID to the corresponding result, if it's also a qubit.
                                if (index < op->getNumResults()) {
                                    Value result = op->getResult(index);
                                    if (result.getType().isa<QubitType>()) {
                                        qubitsInitToOpRes[result] = initSSA;
                                    }
                                }
                            }
                        }
                    }
                }
            });

            // AsmState state(module);
            for (auto &item : qubitsToOps) {
                llvm::StringRef qubitSsaName = item.getKey();
                auto &users = item.second;

                LLVM_DEBUG(llvm::dbgs() << "Gates on qubit " << qubitSsaName << ":\n");
                for (auto *user : users) {
                    LLVM_DEBUG(llvm::dbgs() << "    -> " << user->getName().getStringRef() << "\n");
                }
            }
        }
    }

} // namespace qoala::analysis::gatecount
