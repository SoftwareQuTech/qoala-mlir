#include "Analysis/QMem/Conversion.h"
#include "Analysis/Helpers/Helpers.h"
#include "Analysis/Helpers/QMemInterfaces.h"
#include "llvm/ADT/TypeSwitch.h"
#include "llvm/Support/Debug.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"

#include <set>

#define DEBUG_TYPE "op-classifier"

using namespace mlir;

namespace qoala::analysis::functionize {
    static bool operationBelongsToQMemDialect(const Operation &op) {
        return llvm::isa<
#define GET_OP_LIST
#include "Dialect/QMem/QMem.cpp.inc"
        >(op);
    }

    static bool qMemOpIsaBreakingPoint(const Operation &op) {
        return llvm::isa<
                dialects::qmem::RecvFloatsOp,
                dialects::qmem::RecvIntsOp
                >(op);
    }

    static bool qMemOpIsEprs(const Operation &op) {
        return llvm::isa<
                dialects::qmem::EprsOp,
                dialects::qmem::EprsMeasureOp
                >(op);
    }

    static bool qMemOpShouldRemainInBody(const Operation &op) {
        return llvm::isa<
                dialects::qmem::SendIntsOp,
                dialects::qmem::SendFloatsOp,
                dialects::qmem::FuncOp,
                dialects::qmem::ReturnOp,
                dialects::qmem::RemoteOp
                >(op);
    }

    class PerQubitGroups {
    public:
        PerQubitGroups() = default;
        ~PerQubitGroups();
        [[nodiscard]]
        uint32_t assignQubitIDForQAllocOp(Operation *qallocOp);
        [[nodiscard]]
        std::string addNewGroupForQalloc(Operation *qalloc, const std::vector<uint32_t> &qubits);
        [[nodiscard]]
        QuantumOpsGroupTy *getGroupForQubits(const std::vector<Operation *> &operations);
        [[nodiscard]]
        std::vector<QuantumOpsGroupTy> getAllFinalGroups();
        void commitAllCurrentGroups();
    private:
        [[nodiscard]]
        uint32_t getQubitIDForOperation(Operation *qallocOp) const;
        static std::string getGroupNameForIDs(const std::vector<uint32_t> &qubitIDs);
    private:
        std::map<Operation *, uint32_t> qubitIds;
        std::vector<QuantumOpsGroupTy *> commitedGroups;
        std::map<std::string, QuantumOpsGroupTy *> activeGroups;
    };

    PerQubitGroups::~PerQubitGroups() {
        for (const QuantumOpsGroupTy * group: commitedGroups) {
            delete group;
        }
    }

    std::string PerQubitGroups::getGroupNameForIDs(const std::vector<uint32_t> &qubitIDs) {
        // TODO - Figure out a way to cleverly assign an id to the qubitIDs
        return helpers::formatVector(qubitIDs);
    }

    uint32_t PerQubitGroups::assignQubitIDForQAllocOp(Operation *qallocOp) {
        assert(this->qubitIds.find(qallocOp) == this->qubitIds.end());
        const uint32_t nextAvailableID = this->qubitIds.size();
        this->qubitIds[qallocOp] = nextAvailableID;
        return nextAvailableID;
    }

    std::string PerQubitGroups::addNewGroupForQalloc(Operation *qalloc, const std::vector<uint32_t> &qubits) {
        const std::string groupName = getGroupNameForIDs(qubits);
        auto *newGroup = new QuantumOpsGroupTy;
        this->activeGroups.try_emplace(groupName, newGroup);
        newGroup->push_back(qalloc);
        return groupName;
    }

    uint32_t PerQubitGroups::getQubitIDForOperation(Operation *qallocOp) const {
        assert(this->qubitIds.find(qallocOp) != this->qubitIds.end());
        return this->qubitIds.at(qallocOp);
    }

    QuantumOpsGroupTy *PerQubitGroups::getGroupForQubits(const std::vector<Operation *> &operations) {
        std::vector<uint32_t> qubitIDs;
        for (Operation *qallocOp : operations) {
            qubitIDs.push_back(this->getQubitIDForOperation(qallocOp));
        }
        const std::string &groupName = getGroupNameForIDs(qubitIDs);
        // Two cases
        if (this->activeGroups.find(groupName) != this->activeGroups.end()) {
            // There is an active group for the group ID
            return this->activeGroups.at(groupName);
        }
        // There is not an active qubit; since this operation
        // For how this function is used, we assume that the operation involving the given
        // qubits *is not* a qalloc
        // We need to commit all active current groups, and create a new one
        this->commitAllCurrentGroups();
        auto *newGroup = new QuantumOpsGroupTy;
        this->activeGroups.try_emplace(groupName, newGroup);
        return newGroup;
    }

    void PerQubitGroups::commitAllCurrentGroups() {
        for (const auto &[qubitID, group]: this->activeGroups) {
            this->commitedGroups.push_back(group);
        }
        this->activeGroups.clear();
    }

    std::vector<QuantumOpsGroupTy> PerQubitGroups::getAllFinalGroups() {
        this->commitAllCurrentGroups();
        std::vector<QuantumOpsGroupTy> result;
        result.reserve(this->commitedGroups.size());
        for (QuantumOpsGroupTy *group: this->commitedGroups) {
            result.push_back(std::move(*group));
        }
        return result;
    }

    struct QubitsGroupMap {
        std::set<Operation *> qubitsHandled;
        std::map<Operation *, uint32_t> qubitsGroupIdMap;
        std::map<uint32_t, std::vector<QuantumOpsGroupTy>> operationGroups;
        int32_t currentLocalQuantumOpsGroup = -1;

        void attachNewQubitToLocalOpsGroups(Operation *localQubitOp) {
            assert(qubitsHandled.find(localQubitOp) == qubitsHandled.end());
            if (currentLocalQuantumOpsGroup < 0) {
                // There is no current local quantum ops group; we will create a new one, whose ID will
                // be the current size of the operations group
                currentLocalQuantumOpsGroup = static_cast<int32_t>(operationGroups.size());
                qubitsGroupIdMap.insert(std::pair{localQubitOp, currentLocalQuantumOpsGroup});
                const QuantumOpsGroupTy newEmptyGroup;
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
            const uint32_t groupID = qubitsGroupIdMap[qubitOp];
            return *operationGroups[groupID].rbegin();
        }

        void mapNewQubit(Operation *qubitOp) {
            assert(qubitsHandled.find(qubitOp) == qubitsHandled.end());
            uint32_t newGroupID = operationGroups.size();
            qubitsGroupIdMap.insert(std::pair{qubitOp, newGroupID});
            const QuantumOpsGroupTy newGroup{qubitOp};
            auto entry = std::pair{newGroupID, std::vector<QuantumOpsGroupTy>{newGroup}};
            operationGroups.insert(entry);
        }

        void commitAllCurrentGroups() {
            for (auto [qubitID, qubitGroups]: operationGroups) {
                qubitGroups.emplace_back();
            }
        }

        [[nodiscard]]
        std::vector<QuantumOpsGroupTy> getAllFinalGroups() const {
            uint32_t groupNum = 0;
            std::vector<QuantumOpsGroupTy> opsGroups;
            for (auto [qubitID, qubitGroups]: operationGroups) {
                for (QuantumOpsGroupTy operationsGroup: qubitGroups) {
                    if (!operationsGroup.empty()) {
                        LLVM_DEBUG(llvm::dbgs() << " - Discovered Group #" << groupNum++ << " :\n");
                        for (const Operation *operation: operationsGroup) {
                            LLVM_DEBUG(llvm::dbgs() << "   - op: " << *operation << "\n");
                        }
                        QuantumOpsGroupTy newOpGroup;
                        opsGroups.emplace_back(operationsGroup.begin(), operationsGroup.end());
                    }
                }
                qubitGroups.clear();
            }
            return opsGroups;
        }
    };

    static std::set<Operation *> getEprsQubitOps(dialects::qmem::FuncOp &mainFunction) {
        std::set<Operation *> eprsQubits;
        for (dialects::qmem::EprsOp eprsOp : mainFunction.getOps<dialects::qmem::EprsOp>()) {
            eprsQubits.insert(eprsOp.getQ().getDefiningOp());
        }
        for (dialects::qmem::EprsMeasureOp eprsMeasureOp : mainFunction.getOps<dialects::qmem::EprsMeasureOp>()) {
            eprsQubits.insert(eprsMeasureOp.getQ().getDefiningOp());
        }
        return eprsQubits;
    }

    std::vector<QuantumOpsGroupTy> functionizeOpClassifier(dialects::qmem::FuncOp &mainFunction) {
        PerQubitGroups qubitGroupsMap;
        //QubitsGroupMap qubitGroupsMap;
        std::set<Operation *> eprsQubits = getEprsQubitOps(mainFunction);

        LLVM_DEBUG(llvm::dbgs() << "%%%%%%%%%%%%%%%%%%%%%%%%\n");
        LLVM_DEBUG(llvm::dbgs() << "%      CLASSIFIER      %\n");
        LLVM_DEBUG(llvm::dbgs() << "%%%%%%%%%%%%%%%%%%%%%%%%\n");
        // Iterate over all the operations of the main function
        for (Operation &op : mainFunction.getOps()) {
            LLVM_DEBUG(llvm::dbgs() << " Classifying op = " << op << "\n");
            if (!operationBelongsToQMemDialect(op) || qMemOpShouldRemainInBody(op)) {
                // Operation is not QMem or it was already grouped
                LLVM_DEBUG(llvm::dbgs() << " Discarding (not grouping) op = " << op << "\n");
                continue;
            }
            if (qMemOpIsaBreakingPoint(op)) {
                // The current operation acts as a "barrier", so we start a new, empty group
                // for each of the involved qubits
                // Additionally, it should stay in the main body...
                qubitGroupsMap.commitAllCurrentGroups();
                continue;
            }
            // We get the involved qubits of the operation
            auto qubitOp = dyn_cast<helpers::OpQubitsInterface>(op);
            std::vector<Operation *> involvedQubits = qubitOp.getOpsAllocatingUsedQubits();
            assert(!involvedQubits.empty());

            if (llvm::isa<dialects::qmem::QAllocOp>(op)) {
                // The op is a qalloc
                uint32_t qubitIDForEprsQubits = qubitGroupsMap.assignQubitIDForQAllocOp(&op);
                (void) qubitGroupsMap.addNewGroupForQalloc(&op, {qubitIDForEprsQubits});
            } else {
                // Similar as before, we need to *get a reference* of the group to insert the operation
                // If we don't declare the group as a reference, then *it gets copied* into the local variable
                // so the operation will not be inserted in the respective group
                //LLVM_DEBUG(llvm::dbgs() << " that op : " << *baseQubitOperation << "\n");
                QuantumOpsGroupTy *currentOpsGroup = qubitGroupsMap.getGroupForQubits(involvedQubits);
                // We insert the operation in the group
                currentOpsGroup->push_back(&op);
                // If the operation is EPRS, it also acts as a barrier
                if (qMemOpIsEprs(op)) {
                    qubitGroupsMap.commitAllCurrentGroups();
                }
            }
        }

        // After iterating all the instructions, commit all the discovered groups
        return qubitGroupsMap.getAllFinalGroups();
    }
}
