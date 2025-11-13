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
using namespace qoala::helpers;
using namespace qoala::dialects::qnet;

namespace qoala::analysis::gatecount {

    static bool isTwoQubitOp(QubitOpIface op) {
        // Check if the op is a two-qubit op.
        return op.getOperation()->getNumResults() > 1;
    }

    QNetGateCount::QNetGateCount(Operation *op) {
        // Walk over all ops in the module and count quantum operations.
        // Assign unique Id to qubits at initialization
        // Id is used to track qubits and assign quantum operations to them.

        // Map from quantum operations results to their init Id
        llvm::DenseMap<Value, uint32_t> opResToId;

        auto module = dyn_cast_or_null<ModuleOp>(*op);
        assert(module && "No ModuleOp: something went wrong.");

        uint32_t qId = 0;

        // Walk over all ops in the module.
        module.walk([&](Operation *op) {
            if (auto qubitOp = llvm::dyn_cast<QubitOpIface>(op)) {
                if (llvm::isa<NewQubitOp, EprsOp>(qubitOp)) {
                    for (Value result : qubitOp.getQubitResults()) {
                        opResToId[result] = qId;
                        LLVM_DEBUG(llvm::dbgs() << "New Qubit Op " << qubitOp.getName().getStringRef()
                                                << " for qubit: " << qId << ".\n");
                        detailedOneQubitGateCount[qId] = 0;
                        detailedTwoQubitGateCount[qId] = 0;
                        detailedGateCount[qId] = 0;
                        ++qId;
                    }
                } else {
                    // if (isQuantumOp(op) && !llvm::isa<MeasureOp>(op)) {
                    ++gateCount;
                    if (isTwoQubitOp(qubitOp)) {
                        LLVM_DEBUG(llvm::dbgs() << "Two Qubit Op: " << qubitOp.getName().getStringRef() << ".\n");
                        ++twoQubitGateCount;
                    } else {
                        LLVM_DEBUG(llvm::dbgs() << "One Qubit Op: " << qubitOp.getName().getStringRef() << ".\n");
                        ++oneQubitGateCount;
                    }
                    // For each operand, find its init Id and propagate it to the corresponding result.
                    // As per qnet definition of quantum ops, #results <= #operands and qubits are always the first
                    // operands. Also order of operands and results should be the same, for SSA purposes. Loop through
                    // operands, and for each operand that we are already tracking, i.e. a qubit, find its corresponding
                    // results at the same position.
                    for (auto operandIt : llvm::enumerate(qubitOp.getQubitOperands())) {
                        Value operand = operandIt.value();
                        // Qubit operand index
                        uint32_t index = operandIt.index();

                        // Check if the operand is a qubit we are tracking
                        if (opResToId.contains(operand)) {
                            uint32_t initId = opResToId.at(operand);

                            // Add this operation to the history of the qubit it acts on.
                            LLVM_DEBUG(llvm::dbgs() << "Op " << qubitOp.getName().getStringRef()
                                                    << " on qubit: " << initId << ".\n");
                            detailedGateCount[initId]++;
                            if (isTwoQubitOp(qubitOp)) {
                                detailedTwoQubitGateCount[initId]++;
                            } else {
                                detailedOneQubitGateCount[initId]++;
                            }

                            // Propagate the init Id to the corresponding result.
                            // Resut index should be same as corresponding operand.
                            Value result = qubitOp.getQubitResult(index);
                            opResToId[result] = initId;
                        }
                    }
                    // }
                }
            }
        });
    }

} // namespace qoala::analysis::gatecount
