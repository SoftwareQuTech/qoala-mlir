#include "Analysis/QNet/Helpers.h"
#include "Dialect/QNet/Passes.h"
#include "Dialect/QNet/QNet.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/Debug.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/AsmState.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"

#define DEBUG_TYPE "qnet-gate-count"

using namespace mlir;
using namespace qoala::helpers;
using namespace qoala::dialects::qnet;

namespace qoala::analysis::qnet::gatecount {

    struct BranchCounts {
        uint32_t gateCount;
        uint32_t oneQubitGateCount;
        uint32_t twoQubitGateCount;
        llvm::DenseMap<uint32_t, uint32_t> detailedGateCount;
        llvm::DenseMap<uint32_t, uint32_t> detailedOneQubitGateCount;
        llvm::DenseMap<uint32_t, uint32_t> detailedTwoQubitGateCount;
        llvm::DenseMap<Value, uint32_t> opResToId;

        // Dump branch state into variables
        void dumpInto(uint32_t &outGateCount, uint32_t &outOneQubitGateCount, uint32_t &outTwoQubitGateCount,
                      llvm::DenseMap<uint32_t, uint32_t> &outDetailedGateCount,
                      llvm::DenseMap<uint32_t, uint32_t> &outDetailedOneQubitGateCount,
                      llvm::DenseMap<uint32_t, uint32_t> &outDetailedTwoQubitGateCount,
                      llvm::DenseMap<Value, uint32_t> &outOpResToId) const {
            outGateCount = gateCount;
            outOneQubitGateCount = oneQubitGateCount;
            outTwoQubitGateCount = twoQubitGateCount;
            outDetailedGateCount = detailedGateCount;
            outDetailedOneQubitGateCount = detailedOneQubitGateCount;
            outDetailedTwoQubitGateCount = detailedTwoQubitGateCount;
            outOpResToId = opResToId;
        }
    };

    void visitScfIfOp(scf::IfOp ifOp, llvm::DenseMap<Value, uint32_t> &opResToId, uint32_t &gateCount,
                      uint32_t &oneQubitGateCount, uint32_t &twoQubitGateCount,
                      llvm::DenseMap<uint32_t, uint32_t> &detailedGateCount,
                      llvm::DenseMap<uint32_t, uint32_t> &detailedOneQubitGateCount,
                      llvm::DenseMap<uint32_t, uint32_t> &detailedTwoQubitGateCount) {
        uint32_t qId = 0; // Dummy qId, won't be modified in branches

        // Process then-branch
        BranchCounts thenState{gateCount,         oneQubitGateCount,         twoQubitGateCount,
                               detailedGateCount, detailedOneQubitGateCount, detailedTwoQubitGateCount,
                               opResToId};
        visitOperationsInRegion(ifOp.getThenRegion(), thenState.opResToId, qId, thenState.gateCount,
                                thenState.oneQubitGateCount, thenState.twoQubitGateCount, thenState.detailedGateCount,
                                thenState.detailedOneQubitGateCount, thenState.detailedTwoQubitGateCount);

        // Process else-branch
        BranchCounts elseState{gateCount,         oneQubitGateCount,         twoQubitGateCount,
                               detailedGateCount, detailedOneQubitGateCount, detailedTwoQubitGateCount,
                               opResToId};
        if (!ifOp.getElseRegion().empty()) {
            visitOperationsInRegion(ifOp.getElseRegion(), elseState.opResToId, qId, elseState.gateCount,
                                    elseState.oneQubitGateCount, elseState.twoQubitGateCount,
                                    elseState.detailedGateCount, elseState.detailedOneQubitGateCount,
                                    elseState.detailedTwoQubitGateCount);
        }

        // Select the branch with higher gate count (worst-case) and apply it
        bool thenWins = thenState.gateCount >= elseState.gateCount;
        (thenWins ? thenState : elseState)
                .dumpInto(gateCount, oneQubitGateCount, twoQubitGateCount, detailedGateCount, detailedOneQubitGateCount,
                          detailedTwoQubitGateCount, opResToId);

        // Map the scf.if result values to the qubit IDs from the selected branch's yields
        Region &selectedRegion = thenWins ? ifOp.getThenRegion() : ifOp.getElseRegion();
        auto yields = selectedRegion.front().getTerminator()->getOperands();
        for (auto [ifOpResult, yield] : llvm::zip(ifOp.getResults(), yields)) {
            if (opResToId.contains(yield)) {
                opResToId[ifOpResult] = opResToId[yield];
            }
        }
    }

    void visitOperationsInRegion(Region &region, llvm::DenseMap<Value, uint32_t> &opResToId, uint32_t &qId,
                                 uint32_t &gateCount, uint32_t &oneQubitGateCount, uint32_t &twoQubitGateCount,
                                 llvm::DenseMap<uint32_t, uint32_t> &detailedGateCount,
                                 llvm::DenseMap<uint32_t, uint32_t> &detailedOneQubitGateCount,
                                 llvm::DenseMap<uint32_t, uint32_t> &detailedTwoQubitGateCount) {
        if (region.empty()) {
            return;
        }

        for (auto &op : region.front().getOperations()) {
            if (auto ifOp = dyn_cast<scf::IfOp>(&op)) {
                // Special handling for scf.if
                visitScfIfOp(ifOp, opResToId, gateCount, oneQubitGateCount, twoQubitGateCount, detailedGateCount,
                             detailedOneQubitGateCount, detailedTwoQubitGateCount);
            } else if (auto qubitOp = dyn_cast<QubitOpIface>(&op)) {
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
                    ++gateCount;
                    if (qubitOp.isTwoQubitOp()) {
                        LLVM_DEBUG(llvm::dbgs() << "Two Qubit Op: " << qubitOp.getName().getStringRef() << ".\n");
                        ++twoQubitGateCount;
                    } else {
                        LLVM_DEBUG(llvm::dbgs() << "One Qubit Op: " << qubitOp.getName().getStringRef() << ".\n");
                        ++oneQubitGateCount;
                    }
                    // For each operand, find its init Id and propagate it to the corresponding result.
                    for (auto [operand, result] : llvm::zip(qubitOp.getQubitOperands(), qubitOp.getQubitResults())) {
                        // Check if the operand is a qubit we are tracking
                        assert(opResToId.contains(operand) && "Qubit used but never initialized.");
                        uint32_t initId = opResToId.at(operand);

                        // Add this operation to the history of the qubit it acts on.
                        LLVM_DEBUG(llvm::dbgs()
                                   << "Op " << qubitOp.getName().getStringRef() << " on qubit: " << initId << ".\n");
                        detailedGateCount[initId]++;
                        if (qubitOp.isTwoQubitOp()) {
                            detailedTwoQubitGateCount[initId]++;
                        } else {
                            detailedOneQubitGateCount[initId]++;
                        }

                        // Propagate the init Id to the corresponding result.
                        opResToId[result] = initId;
                    }
                }
            }
        }
    }

    QNetGateCount::QNetGateCount(Operation *op) {
        /**
         * Walk over all ops in the module and count quantum operations.
         * Assign unique Id to qubits at initialization
         * Id is used to track qubits and assign quantum operations to them.
         */

        // Map from quantum operations results to their init Id
        llvm::DenseMap<Value, uint32_t> opResToId;

        auto module = dyn_cast_or_null<ModuleOp>(*op);
        assert(module && "No ModuleOp: something went wrong.");

        uint32_t qId = 0;

        // Find the qnet.func operation and traverse its body
        auto funcOps = module.getOps<qoala::dialects::qnet::FuncOp>();
        assert(!funcOps.empty() && "No qnet.func operation found");
        auto funcOp = *funcOps.begin();
        visitOperationsInRegion(funcOp.getRegion(), opResToId, qId, gateCount, oneQubitGateCount, twoQubitGateCount,
                                detailedGateCount, detailedOneQubitGateCount, detailedTwoQubitGateCount);
    }

} // namespace qoala::analysis::qnet::gatecount
