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

    /**
     * Class used to group operations based on the used qubit.
     * This class will group together all operations that are *lexicographically next
     * to each other*. This means that a sequence of operations like this:
     * %q0 = qmem.qalloc : i32
     * qmem.init %q0
     * %q1 = qmem.qalloc : i32
     * qmem.init %q1
     * qmem.hadamard %q0
     * qmem.hadamard %1
     * will generate 4 groups: [(q0, qalloc+init), (q1, qalloc+init), (q0, hadamard), (q1, hadamard).
     * In a similar manner, operations that use more than 1 qubit, will yield a new group:
     * %q0 = qmem.qalloc : i32
     * qmem.init %q0
     * %q1 = qmem.qalloc : i32
     * qmem.init %q1
     * qmem.cnot %q0, %q1
     * which generates 3 groups: [(q0, qalloc+init), (q1, qalloc+init), (q0+q1, cnot)]
     */
    class PerQubitGrouper {
    public:
        PerQubitGrouper() = default;
        ~PerQubitGrouper();
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

    PerQubitGrouper::~PerQubitGrouper() {
        for (const QuantumOpsGroupTy * group: commitedGroups) {
            delete group;
        }
    }

    std::string PerQubitGrouper::getGroupNameForIDs(const std::vector<uint32_t> &qubitIDs) {
        return helpers::formatVector(qubitIDs);
    }

    uint32_t PerQubitGrouper::assignQubitIDForQAllocOp(Operation *qallocOp) {
        assert(this->qubitIds.find(qallocOp) == this->qubitIds.end());
        const uint32_t nextAvailableID = this->qubitIds.size();
        this->qubitIds[qallocOp] = nextAvailableID;
        return nextAvailableID;
    }

    std::string PerQubitGrouper::addNewGroupForQalloc(Operation *qalloc, const std::vector<uint32_t> &qubits) {
        const std::string groupName = getGroupNameForIDs(qubits);
        auto *newGroup = new QuantumOpsGroupTy;
        this->activeGroups.try_emplace(groupName, newGroup);
        newGroup->push_back(qalloc);
        return groupName;
    }

    uint32_t PerQubitGrouper::getQubitIDForOperation(Operation *qallocOp) const {
        assert(this->qubitIds.find(qallocOp) != this->qubitIds.end());
        return this->qubitIds.at(qallocOp);
    }

    QuantumOpsGroupTy *PerQubitGrouper::getGroupForQubits(const std::vector<Operation *> &operations) {
        std::vector<uint32_t> qubitIDs;
        qubitIDs.reserve(operations.size());
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
        // We need to commit all active current groups, and create a new one. If we don't
        // commit active groups, a subsequent operation on a qubit that has an active group
        // would end up in that group. That behavior is equivalent to move that operation *before*
        // the current operation.
        this->commitAllCurrentGroups();
        auto *newGroup = new QuantumOpsGroupTy;
        this->activeGroups.try_emplace(groupName, newGroup);
        return newGroup;
    }

    void PerQubitGrouper::commitAllCurrentGroups() {
        for (const auto &[qubitID, group]: this->activeGroups) {
            this->commitedGroups.push_back(group);
        }
        this->activeGroups.clear();
    }

    std::vector<QuantumOpsGroupTy> PerQubitGrouper::getAllFinalGroups() {
        this->commitAllCurrentGroups();
        std::vector<QuantumOpsGroupTy> result;
        result.reserve(this->commitedGroups.size());
        for (QuantumOpsGroupTy *group: this->commitedGroups) {
            result.push_back(std::move(*group));
        }
        return result;
    }

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
        PerQubitGrouper qubitGroupsMap;
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
                // The op is a qalloc; we assign a new qubit ID fir this operation
                uint32_t qubitIDForEprsQubits = qubitGroupsMap.assignQubitIDForQAllocOp(&op);
                // We also assign a new bin for the qubit allocated. This also add the qalloc op in the new bin
                (void) qubitGroupsMap.addNewGroupForQalloc(&op, {qubitIDForEprsQubits});
            } else {
                // In the case of a non-qalloc, we need to get the corresponding group for the
                // qubits involved in the operation.
                QuantumOpsGroupTy *currentOpsGroup = qubitGroupsMap.getGroupForQubits(involvedQubits);
                // We insert the operation in the group
                currentOpsGroup->push_back(&op);
                // If the operation is EPRS, it also acts as a barrier, so we commit current active groups.
                if (qMemOpIsEprs(op)) {
                    qubitGroupsMap.commitAllCurrentGroups();
                }
            }
        }

        // After iterating all the instructions, commit all the discovered groups
        return qubitGroupsMap.getAllFinalGroups();
    }
}
