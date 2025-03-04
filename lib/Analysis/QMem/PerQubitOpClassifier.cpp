#include "Analysis/QMem/Conversion.h"
#include "Analysis/Helpers/QMemInterfaces.h"
#include "llvm/ADT/TypeSwitch.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "op-classifier"

using namespace mlir;

namespace qoala::analysis::functionize {
    static bool operationBelongsToQMemDialect(Operation &op) {
        return llvm::isa<
#define GET_OP_LIST
#include "Dialect/QMem/QMem.cpp.inc"
        >(op);
    }

    static bool qMemOpIsaBreakingPoint(Operation &op) {
        return llvm::isa<
                dialects::qmem::RecvFloatsOp,
                dialects::qmem::RecvIntsOp
        >(op);
    }

    static bool qMemOpIsEprs(Operation &op) {
        return llvm::isa<
                dialects::qmem::EprsOp,
                dialects::qmem::EprsMeasureOp
        >(op);
    }

    static bool qMemOpShouldRemainInBody(Operation &op) {
        return llvm::isa<
                dialects::qmem::SendIntsOp,
                dialects::qmem::SendFloatsOp,
                dialects::qmem::FuncOp,
                dialects::qmem::ReturnOp,
                dialects::qmem::RemoteOp
        >(op);
    }

    struct QubitsGroupMap {
        std::set<Operation *> qubitsHandled;
        std::map<Operation *, unsigned int> qubitsGroupIdMap;
        std::map<unsigned int, std::vector<QuantumOpsGroupTy>> operationGroups;
        long currentLocalQuantumOpsGroup = -1;

        void attachNewQubitToLocalOpsGroups(Operation *localQubitOp) {
            assert(!qubitsHandled.contains(localQubitOp));
            if (currentLocalQuantumOpsGroup < 0) {
                // There is no current local quantum ops group; we will create a new one, whose ID will
                // be the current size of the operations group
                currentLocalQuantumOpsGroup = (long) operationGroups.size();
                qubitsGroupIdMap.insert(std::pair{localQubitOp, currentLocalQuantumOpsGroup});
                QuantumOpsGroupTy newEmptyGroup;
                auto entry = std::pair{currentLocalQuantumOpsGroup, std::vector<QuantumOpsGroupTy>{newEmptyGroup}};
                operationGroups.insert(entry);
            }
            // We attach the operation to the current local quantum ops group
            qubitsHandled.insert(localQubitOp);
            QuantumOpsGroupTy &lastLocalOpsGroup = *operationGroups[currentLocalQuantumOpsGroup].rbegin();
            lastLocalOpsGroup.push_back(localQubitOp);
        }

        /// Returns the last operations group for the given qubit operation
        QuantumOpsGroupTy &operator[](Operation *qubitOp) {
            unsigned int groupID = qubitsGroupIdMap[qubitOp];
            return *operationGroups[groupID].rbegin();
        }

        void mapNewQubit(Operation *qubitOp) {
            assert(!qubitsHandled.contains(qubitOp));
            unsigned int newGroupID = operationGroups.size();
            qubitsGroupIdMap.insert(std::pair{qubitOp, newGroupID});
            QuantumOpsGroupTy newGroup{qubitOp};
            auto entry = std::pair{newGroupID, std::vector<QuantumOpsGroupTy>{newGroup}};
            operationGroups.insert(entry);
        }

        void commitAllCurrentGroups() {
            for (auto &operationGroupsEntry : operationGroups) {
                operationGroupsEntry.second.emplace_back();
            }
        }

        std::vector<QuantumOpsGroupTy> getAllFinalGroups() {
            unsigned int groupNum = 0;
            std::vector<QuantumOpsGroupTy> opsGroups;
            for (auto qubitGroupsEntry: operationGroups) {
                for (QuantumOpsGroupTy operationsGroup: qubitGroupsEntry.second) {
                    if (!operationsGroup.empty()) {
                        LLVM_DEBUG(llvm::dbgs() << " - Discovered Group #" << groupNum++ << " :\n");
                        for (Operation *operation: operationsGroup) {
                            LLVM_DEBUG(llvm::dbgs() << "   - op: " << *operation << "\n");
                        }
                        QuantumOpsGroupTy newOpGroup;
                        opsGroups.emplace_back(operationsGroup.begin(), operationsGroup.end());
                    }
                }
                qubitGroupsEntry.second.clear();
            }
            return opsGroups;
        }
    };

    static std::set<Operation *> getEprsQubitOps(dialects::qmem::FuncOp &mainFunction) {
        std::set<Operation *> eprsQubits;
        auto eprsOps = mainFunction.getOps<dialects::qmem::EprsOp>();
        auto eprsMeasureOps = mainFunction.getOps<dialects::qmem::EprsMeasureOp>();

        for (dialects::qmem::EprsOp eprsOp : eprsOps) {
            eprsQubits.insert(eprsOp.getQ().getDefiningOp());
        }
        for (dialects::qmem::EprsMeasureOp eprsMeasureOp : eprsMeasureOps) {
            eprsQubits.insert(eprsMeasureOp.getQ().getDefiningOp());
        }
        return eprsQubits;
    }

    std::vector<QuantumOpsGroupTy> functionizeOpClassifier(dialects::qmem::FuncOp &mainFunction) {
        QubitsGroupMap qubitGroupsMap;
        std::set<Operation *> eprsQubits = getEprsQubitOps(mainFunction);

        LLVM_DEBUG(llvm::dbgs() << "%%%%%%%%%%%%%%%%%%%%%%%%\n");
        LLVM_DEBUG(llvm::dbgs() << "%      CLASSIFIER      %\n");
        LLVM_DEBUG(llvm::dbgs() << "%%%%%%%%%%%%%%%%%%%%%%%%\n");
        // Iterate over all the operations of the main function
        for (Operation &op : mainFunction.getOps()) {
            LLVM_DEBUG(llvm::dbgs() << " Classifying op = " << op << "\n");
            if (!operationBelongsToQMemDialect(op) || qMemOpShouldRemainInBody(op)) {
                // Operation is not QMem or it was already grouped
                continue;
            }
            if (qMemOpIsaBreakingPoint(op)) {
                // The current operation acts as a "barrier", so we start a new, empty group
                // for each of the involved qubits
                // Additionally, it should stay in the main body...
                qubitGroupsMap.commitAllCurrentGroups();
                continue;
            } else {
                // We get the involved qubits of the operation
                auto qubitOp = dyn_cast<qoala::helpers::OpQubitsInterface>(op);
                std::vector<Operation *> involvedQubits = qubitOp.getOpsAllocatingUsedQubits();
                assert(!involvedQubits.empty());

                if (llvm::isa<dialects::qmem::QAllocOp>(op)) {
                    // The op is a qalloc
                    if (eprsQubits.contains(&op)) {
                        // We start a new group for this qubit iff it is used for eprs
                        qubitGroupsMap.mapNewQubit(&op);
                    } else {
                        // Otherwise, we attach this "local" qubit to the current group of local ops
                        qubitGroupsMap.attachNewQubitToLocalOpsGroups(&op);
                    }
                } else {
                    // We choose one of the involved qubits to attach this op to its group. In particular
                    // the first qubit that appears lexicographically
                    Operation *baseQubitOperation = involvedQubits[0];
                    // Similar as before, we need to *get a reference* of the group to insert the operation
                    // If we don't declare the group as a reference, then *it gets copied* into the local variable
                    // so the operation will not be inserted in the respective group
                    QuantumOpsGroupTy &currentOpsGroup = qubitGroupsMap[baseQubitOperation];
                    // We insert the operation in the group
                    currentOpsGroup.push_back(&op);
                    // If the operation is EPRS, it also acts as a barrier
                    if (qMemOpIsEprs(op)) {
                        qubitGroupsMap.commitAllCurrentGroups();
                    }
                }
            }
        }

        // After iterating all the instructions, commit all the discovered groups
        return qubitGroupsMap.getAllFinalGroups();
    }
}
