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

    static bool isQuantumOp(mlir::Operation *op) {
        // Check if the op is a quantum operation.
        return llvm::isa<NewQubitOp, RotXOp, RotYOp, RotZOp, HadamardOp, CnotOp, CzOp, CrotXOp, MeasureOp, EprsOp,
                         EprsMeasureOp>(op);
    }

    static bool isTwoQubitOp(mlir::Operation *op) {
        // Check if the op is a two-qubit op.
        return llvm::isa<CnotOp, CzOp, CrotXOp>(op);
    }

    static std::string getSSAName(Value val, AsmState &state) {
        // Get the SSA name of the value with respect to the asm state.
        std::string ssaName;
        llvm::raw_string_ostream stream(ssaName);
        val.printAsOperand(stream, state);

        return ssaName;
    }

    QNetGateCount::QNetGateCount(Operation *op) {
        // Walk over all ops in the module and count quantum operations.
        // Keep track of SSA names to assign quantum operations
        // to the right qubit.

        // Map from qubits SSA name at their creation to all quantum operation applied to them
        llvm::StringMap<std::vector<Operation *>> qubitsToOps;
        // Map from quantum operations results SSA names to their SSA name at creation
        llvm::DenseMap<Value, std::string> opResToInitSSA;

        if (ModuleOp module = llvm::dyn_cast<ModuleOp>(op)) {

            AsmState state(module);

            // Walk over all ops in the module.
            module.walk([&](Operation *op) {
                if (llvm::isa<NewQubitOp, EprsOp>(op)) {
                    for (Value result : op->getResults()) {
                        std::string qubitSsaName = getSSAName(result, state);
                        LLVM_DEBUG(llvm::dbgs() << "New Qubit Op " << op->getName().getStringRef()
                                                << " for qubit: " << qubitSsaName << ".\n");
                        opResToInitSSA[result] = qubitSsaName;
                        qubitsToOps.try_emplace(qubitSsaName);
                        qubitsToOps[qubitSsaName].push_back(op);
                        detailedOneGateCount[qubitSsaName] = 0;
                        detailedTwoGateCount[qubitSsaName] = 0;
                        detailedGateCount[qubitSsaName] = 0;
                    }
                } else {
                    if (isQuantumOp(op) and !llvm::isa<MeasureOp>(op)) {
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
                            auto initSSAIt = opResToInitSSA.find(operand);
                            if (initSSAIt != opResToInitSSA.end()) {
                                std::string initSSA = initSSAIt->second;
                                LLVM_DEBUG(llvm::dbgs() << "Op " << op->getName().getStringRef()
                                                        << " on qubit: " << initSSA << ".\n");

                                // Add this operation to the history of the qubit it acts on.
                                qubitsToOps[initSSA].push_back(op);
                                detailedGateCount[initSSA]++;
                                if (isTwoQubitOp(op)) {
                                    detailedTwoGateCount[initSSA]++;
                                } else {
                                    detailedOneGateCount[initSSA]++;
                                }

                                // Propagate the SSA name to the corresponding result, if it's also a qubit.
                                if (index < op->getNumResults()) {
                                    Value result = op->getResult(index);
                                    if (result.getType().isa<QubitType>()) {
                                        opResToInitSSA[result] = initSSA;
                                    }
                                }
                            }
                        }
                    }
                }
            });

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
