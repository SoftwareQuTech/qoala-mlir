#include "Analysis/Helpers/Helpers.h"
#include "Analysis/Helpers/QMemInterfaces.h"
#include "Analysis/QMem/Conversion.h"
#include "Tools/QoalaOpt.h"
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
        return llvm::isa<dialects::qmem::RecvFloatsOp, dialects::qmem::RecvIntsOp,
                         dialects::qmem::RecvFloatOp, dialects::qmem::RecvIntOp>(op);
    }

    static bool qMemOpShouldRemainInBody(const Operation &op) {
        return llvm::isa<dialects::qmem::SendIntsOp, dialects::qmem::SendFloatsOp,
                         dialects::qmem::SendIntOp, dialects::qmem::SendFloatOp, dialects::qmem::FuncOp,
                         dialects::qmem::ReturnOp, dialects::qmem::RemoteOp>(op);
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
        PerQubitGrouper() = delete;

        explicit PerQubitGrouper(const uint32_t maxOpsPerGroup):
            maxOpsPerGroup(maxOpsPerGroup), activeGroup(std::nullopt) { }

        ~PerQubitGrouper();
        void registerQallocOp(dialects::qmem::QAllocOp &qallocOp);
        void groupDefinitionWithQAlloc(helpers::DefineQubitsInterface &defOp);
        void addOperationToGroup(Operation *op, const std::vector<Operation *> &involvedQubitOps);
        [[nodiscard]]
        std::vector<QuantumOpsGroupTy> getAllFinalGroups();
        void commitCurrentGroup();

        void groupEprsByRemote(
                const llvm::DenseMap<std::pair<StringRef, StringRef>, std::vector<Operation *>> &opsByRemoteAndKind);

    private:
        [[nodiscard]]
        QuantumOpsGroupTy *getGroupForQubits(const std::vector<Operation *> &operations);
        [[nodiscard]]
        QuantumOpsGroupTy *addNewGroupForQalloc(Operation *qalloc, const std::vector<uint32_t> &qubits);
        [[nodiscard]]
        dialects::qmem::QAllocOp getQallocForQubit(const Value &qubitVal);
        void deregisterQallocOp(dialects::qmem::QAllocOp &qallocOp);
        [[nodiscard]]
        uint32_t assignQubitIDForQAllocOp(Operation *qallocOp);
        [[nodiscard]]
        uint32_t getQubitIDForOperation(Operation *qallocOp) const;
        static std::string getGroupNameForIDs(const std::vector<uint32_t> &qubitIDs);

        bool hasQubitID(Operation *qallocOp) const { return qubitIds.count(qallocOp) > 0; }

        std::pair<std::vector<uint32_t>, std::vector<Operation *>>
        assignRemoteQubitIDsWithQalloc(const std::vector<Operation *> &ops);

    private:
        uint32_t maxOpsPerGroup;
        // The currently handled qubits
        std::map<Operation *, uint32_t> qubitIds;
        // The already-commited groups
        std::vector<QuantumOpsGroupTy *> commitedGroups;
        // To preserve the order of the quantum ops as they appear lexicographically
        // this class maintains references to the *single* current active group and its ID
        std::string activeGroupId;
        std::optional<QuantumOpsGroupTy *> activeGroup;
        // Similar as simple functionize, we need to keep track of "declarations" of qubits
        // to group them with their definitions later.
        DenseMap<Value, dialects::qmem::QAllocOp> declaredQubits;
    };

    PerQubitGrouper::~PerQubitGrouper() {
        for (const QuantumOpsGroupTy *group : commitedGroups) {
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
        assert(this->declaredQubits.contains(qubitVal) &&
               "Per qubit grouper: trying to get the qalloc operation of an unknown qubit");
        return this->declaredQubits[qubitVal];
    }

    void PerQubitGrouper::deregisterQallocOp(dialects::qmem::QAllocOp &qallocOp) {
        assert(this->declaredQubits.contains(qallocOp.getQ()) &&
               "Per qubit grouper: trying to deregister the qalloc operation of an unknown qubit");
        this->declaredQubits.erase(qallocOp.getQ());
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
        this->deregisterQallocOp(qallocOp);
        defOp.getOperation();
    }

    void PerQubitGrouper::addOperationToGroup(Operation *op, const std::vector<Operation *> &involvedQubitOps) {
        // Get the corresponding group
        QuantumOpsGroupTy *currentOpsGroup = this->getGroupForQubits(involvedQubitOps);
        if (this->maxOpsPerGroup > 0 && currentOpsGroup->size() >= this->maxOpsPerGroup) {
            // When the current group is full, we need to commit the current group
            this->commitCurrentGroup();
            // And get a new group for the operation
            currentOpsGroup = this->getGroupForQubits(involvedQubitOps);
        }
        // We insert the operation in the group
        currentOpsGroup->push_back(op);
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
        for (QuantumOpsGroupTy *group : this->commitedGroups) {
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

    std::pair<std::vector<uint32_t>, std::vector<Operation *>>
    PerQubitGrouper::assignRemoteQubitIDsWithQalloc(const std::vector<Operation *> &ops) {
        std::vector<uint32_t> qubitIDs;
        std::vector<Operation *> qallocs;

        for (Operation *op : ops) {
            auto defineIface = dyn_cast<helpers::DefineQubitsInterface>(op);
            assert(defineIface && "Expected entangle op to implement DefineQubitsInterface");

            Operation *qallocOp = defineIface.getDefiningQubit().getDefiningOp();
            // Assign ID only if not already assigned (avoids duplicate ID assignment)
            uint32_t qubitID;
            if (!this->hasQubitID(qallocOp)) {
                qubitID = this->assignQubitIDForQAllocOp(qallocOp);
            } else {
                qubitID = this->getQubitIDForOperation(qallocOp);
            }
            qubitIDs.push_back(qubitID);
            qallocs.push_back(qallocOp);
        };

        return {qubitIDs, qallocs};
    }

    void PerQubitGrouper::groupEprsByRemote(
            const llvm::DenseMap<std::pair<StringRef, StringRef>, std::vector<Operation *>> &opsByRemoteAndKind) {
        for (const auto &[key, ops] : opsByRemoteAndKind) {
            const auto &[remoteName, kind] = key;

            LLVM_DEBUG(llvm::dbgs() << " Grouping entangle ops for remote = " << remoteName << ", kind = " << kind
                                    << "\n");

            auto [qubitIDs, qallocs] = this->assignRemoteQubitIDsWithQalloc(ops);

            // Create a new group for all EPRS ops sharing the same remote
            auto *group = this->addNewGroupForQalloc(qallocs[0], qubitIDs);
            // Insert the qalloc and eprs ops into the group
            for (size_t i = 0; i < ops.size(); ++i) {
                // The first qalloc is already handled during group creation, so skip it
                if (i != 0) {
                    group->push_back(qallocs[i]);
                }
                group->push_back(ops[i]);

                // Clean up: deregister the qalloc from the declared qubits map
                auto qallocTyped = cast<dialects::qmem::QAllocOp>(qallocs[i]);
                this->deregisterQallocOp(qallocTyped);
            }

            // Commit the group so future ops won't merge into it
            this->commitCurrentGroup();
        }
    }

    std::vector<QuantumOpsGroupTy> functionizeOpClassifier(dialects::qmem::FuncOp &mainFunction,
                                                           const uint32_t maxOpsPerGroup) {
        PerQubitGrouper qubitGroupsMap(maxOpsPerGroup);
        std::set<Operation *> eprsQubits = getEprsQubitOps(mainFunction);

        // If grouping by remote is enabled, track EprsOps and EprsMeasureOps by remote
        llvm::DenseMap<std::pair<StringRef, StringRef>, std::vector<Operation *>> entangleOpsGrouped;

        LLVM_DEBUG(llvm::dbgs() << "%%%%%%%%%%%%%%%%%%%%%%%%\n");
        LLVM_DEBUG(llvm::dbgs() << "%      CLASSIFIER      %\n");
        LLVM_DEBUG(llvm::dbgs() << "%%%%%%%%%%%%%%%%%%%%%%%%\n");
        LLVM_DEBUG(llvm::dbgs() << "% using maxOps = " << maxOpsPerGroup << "\n");
        LLVM_DEBUG(llvm::dbgs() << "% grouping entanglement requests = " << qoala::options::qoalaOptGroupEntReqs
                                << "\n");

        for (dialects::qmem::QAllocOp qallocOp : mainFunction.getOps<dialects::qmem::QAllocOp>()) {
            LLVM_DEBUG(llvm::dbgs() << " Classifying alloc op = " << *qallocOp << "\n");
            qubitGroupsMap.registerQallocOp(qallocOp);
        }

        // Collect EPR ops by remote if grouping is enabled
        if (options::qoalaOptGroupEntReqs) {
            mainFunction.walk([&](helpers::EntangleInterface op) {
                StringRef remote = op.getRemote();
                StringRef kind = op->getName().getStringRef();
                entangleOpsGrouped[{remote, kind}].push_back(op.getOperation());
            });

            // Pre-group and commit all EPRS ops by remote before traversing other ops
            qubitGroupsMap.groupEprsByRemote(entangleOpsGrouped);
        }

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
                    .Case([&](dialects::qmem::QAllocOp qallocOp) {})
                    .Case([&](dialects::qmem::InitOp &initOp) {
                        if (auto definingOp = dyn_cast<helpers::DefineQubitsInterface>(initOp.getOperation())) {
                            qubitGroupsMap.groupDefinitionWithQAlloc(definingOp);
                        }
                    })
                    .Case([&](dialects::qmem::EprsOp &eprsOp) {
                        // If grouping was enabled, these were already committed earlier
                        if (!options::qoalaOptGroupEntReqs) {
                            if (auto definingOp = dyn_cast<helpers::DefineQubitsInterface>(eprsOp.getOperation())) {
                                qubitGroupsMap.groupDefinitionWithQAlloc(definingOp);
                            }
                            qubitGroupsMap.commitCurrentGroup();
                        }
                    })
                    .Case([&](dialects::qmem::EprsMeasureOp &eprsOp) {
                        // If grouping was enabled, these were already committed earlier
                        if (!options::qoalaOptGroupEntReqs) {
                            if (auto definingOp = dyn_cast<helpers::DefineQubitsInterface>(eprsOp.getOperation())) {
                                qubitGroupsMap.groupDefinitionWithQAlloc(definingOp);
                            }
                            qubitGroupsMap.commitCurrentGroup();
                        }
                    })
                    .Default([&](Operation *otherOp) {
                        // In the case of a non-qalloc, we need to attach the operation to the
                        // corresponding group. This call will make sure that the size of the current
                        // group does not exceed the configured threshold.
                        qubitGroupsMap.addOperationToGroup(otherOp, {involvedQubits});
                    });
        }

        // After iterating all the instructions, commit all the discovered groups
        return qubitGroupsMap.getAllFinalGroups();
    }
} // namespace qoala::analysis::functionize
