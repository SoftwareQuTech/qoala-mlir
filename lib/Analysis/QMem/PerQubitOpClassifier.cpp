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
        PerQubitGrouper() : activeGroup(std::nullopt) {}
        ~PerQubitGrouper();
        void registerQallocOp(dialects::qmem::QAllocOp &qallocOp);
        void groupDefinitionWithQAlloc(helpers::DefineQubitsInterface &defOp);
        [[nodiscard]]
        QuantumOpsGroupTy *getGroupForQubits(const std::vector<Operation *> &operations);
        [[nodiscard]]
        std::vector<QuantumOpsGroupTy> getAllFinalGroups();
        void commitCurrentGroup();
    private:
        [[nodiscard]]
        QuantumOpsGroupTy *addNewGroupForQalloc(Operation *qalloc, const std::vector<uint32_t> &qubits);
        [[nodiscard]]
        dialects::qmem::QAllocOp getQallocForQubit(const Value &qubitVal);
        [[nodiscard]]
        uint32_t assignQubitIDForQAllocOp(Operation *qallocOp);
        [[nodiscard]]
        uint32_t getQubitIDForOperation(Operation *qallocOp) const;
        static std::string getGroupNameForIDs(const std::vector<uint32_t> &qubitIDs);
    private:
        // The currently handled qubits
        std::map<Operation *, uint32_t> qubitIds;
        // The already-commited groups
        std::vector<QuantumOpsGroupTy *> commitedGroups;
        // To preserve the order of the quantum ops as they appear lexicographically
        // this class maintains references to the *single* current active group and its ID
        std::string activeGroupId;
        std::optional<QuantumOpsGroupTy *>activeGroup;
        // Similar as simple functionize, we need to keep track of "declarations" of qubits
        // to group them with their definitions later.
        DenseMap<Value, dialects::qmem::QAllocOp> declaredQubits;
    };

    PerQubitGrouper::~PerQubitGrouper() {
        for (const QuantumOpsGroupTy * group: commitedGroups) {
            delete group;
        }
        if (this->activeGroup.has_value()) {
            delete this->activeGroup.value();
        }
    }

    std::string PerQubitGrouper::getGroupNameForIDs(const std::vector<uint32_t> &qubitIDs) {
        return helpers::formatVector(qubitIDs);
    }

    dialects::qmem::QAllocOp PerQubitGrouper::getQallocForQubit(const Value &qubitVal) {
        assert(this->declaredQubits.contains(qubitVal) && "Per qubit grouper: trying to get the qalloc operation of an unknown qubit");
        return this->declaredQubits[qubitVal];
    }

    uint32_t PerQubitGrouper::assignQubitIDForQAllocOp(Operation *qallocOp) {
        assert(this->qubitIds.find(qallocOp) == this->qubitIds.end());
        const uint32_t nextAvailableID = this->qubitIds.size();
        this->qubitIds[qallocOp] = nextAvailableID;
        return nextAvailableID;
    }

    void PerQubitGrouper::registerQallocOp(dialects::qmem::QAllocOp &qallocOp) {
        const auto result = this->declaredQubits.try_emplace(qallocOp.getQ(), qallocOp);
        (void) result;
        assert(result.second && "Per qubit grouper: trying to map a qalloc operation that was already mapped");
    }

    void PerQubitGrouper::groupDefinitionWithQAlloc(helpers::DefineQubitsInterface &defOp) {
        // This is an init op, group this operation together with its qubit declaration
        dialects::qmem::QAllocOp qallocOp = this->getQallocForQubit(defOp.getDefiningQubit());
        uint32_t qubitIDForOp = this->assignQubitIDForQAllocOp(qallocOp);
        QuantumOpsGroupTy *qallocGroup = this->addNewGroupForQalloc(qallocOp.getOperation(), {qubitIDForOp});
        qallocGroup->push_back(defOp.getOperation());
        defOp.getOperation();
    }


    QuantumOpsGroupTy *PerQubitGrouper::addNewGroupForQalloc(Operation *qalloc, const std::vector<uint32_t> &qubits) {
        const std::string groupName = getGroupNameForIDs(qubits);
        assert(this->activeGroupId != groupName);
        this->commitCurrentGroup();
        this->activeGroupId = groupName;
        this->activeGroup.emplace(new QuantumOpsGroupTy);
        this->activeGroup.value()->push_back(qalloc);
        return this->activeGroup.value();
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
        if (this->activeGroupId == groupName) {
            // The group is currently active; return it
            return this->activeGroup.value();
        }
        // There is not an active group for the given qubits.
        // For how this function is used, we assume that the operation involving the given
        // qubits *is not* a qalloc
        // We need to commit the active current group, and create a new one.
        this->commitCurrentGroup();
        this->activeGroupId = groupName;
        this->activeGroup.emplace(new QuantumOpsGroupTy);
        return this->activeGroup.value();
    }

    void PerQubitGrouper::commitCurrentGroup() {
        if (this->activeGroup.has_value()) {
            if (this->activeGroup.value()->empty()) {
                delete this->activeGroup.value();
            } else {
                this->commitedGroups.push_back(this->activeGroup.value());
            }
            this->activeGroupId = "";
            this->activeGroup = std::nullopt;
        }
    }

    std::vector<QuantumOpsGroupTy> PerQubitGrouper::getAllFinalGroups() {
        this->commitCurrentGroup();
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
                qubitGroupsMap.commitCurrentGroup();
                continue;
            }
            // We get the involved qubits of the operation
            auto qubitOp = dyn_cast<helpers::OpQubitsInterface>(op);
            std::vector<Operation *> involvedQubits = qubitOp.getOpsAllocatingUsedQubits();
            assert(!involvedQubits.empty());

            llvm::TypeSwitch<Operation *>(&op)
            .Case([&](dialects::qmem::QAllocOp qallocOp) {
                // The op is a qalloc; we register the qalloc op to be grouped later
                qubitGroupsMap.registerQallocOp(qallocOp);
            })
            .Case([&](dialects::qmem::InitOp &initOp) {
                if (auto definingOp = dyn_cast<helpers::DefineQubitsInterface>(initOp.getOperation())) {
                    qubitGroupsMap.groupDefinitionWithQAlloc(definingOp);
                }
            })
            .Case([&](dialects::qmem::EprsOp &eprsOp) {
                if (auto definingOp = dyn_cast<helpers::DefineQubitsInterface>(eprsOp.getOperation())) {
                    qubitGroupsMap.groupDefinitionWithQAlloc(definingOp);
                }
                // If the operation is EPRS, it also acts as a barrier, so we commit current active groups.
                if (qMemOpIsEprs(op)) {
                    qubitGroupsMap.commitCurrentGroup();
                }
            })
            .Case([&](dialects::qmem::EprsMeasureOp &eprsOp) {
                if (auto definingOp = dyn_cast<helpers::DefineQubitsInterface>(eprsOp.getOperation())) {
                    qubitGroupsMap.groupDefinitionWithQAlloc(definingOp);
                }
            })
            .Default([&](Operation *otherOp ) {
                // In the case of a non-qalloc, we need to get the corresponding group for the
                // qubits involved in the operation.
                QuantumOpsGroupTy *currentOpsGroup = qubitGroupsMap.getGroupForQubits(involvedQubits);
                // We insert the operation in the group
                currentOpsGroup->push_back(otherOp);
            });
        }

        // After iterating all the instructions, commit all the discovered groups
        return qubitGroupsMap.getAllFinalGroups();
    }
}
