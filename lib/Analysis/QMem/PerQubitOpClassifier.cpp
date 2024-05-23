#include "Analysis/QMem/Conversion.h"
#include "Analysis/Helpers/QMemInterfaces.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "op-classifier"

namespace qoala::analysis::functionize {
    static bool operationBelongsToQMemDialect(Operation &op) {
        return llvm::isa<
#define GET_OP_LIST
#include "Dialect/QMem/QMem.cpp.inc"
        >(op);
    }

    static bool qMemOpIsClassicalCommunication(Operation &op) {
        return llvm::isa<
                dialects::qmem::RecvFloatsOp,
                dialects::qmem::RecvIntsOp,
                dialects::qmem::SendFloatsOp,
                dialects::qmem::SendIntsOp
        >(op);
    }

    static bool qMemOpInitsQubit(Operation *op) {
        return llvm::isa<
                dialects::qmem::EprsOp,
                dialects::qmem::InitOp,
                dialects::qmem::EprsMeasureOp
        >(op);
    }

    static bool qMemOpShouldRemainInBody(Operation &op) {
        return llvm::isa<
                dialects::qmem::FuncOp,
                dialects::qmem::ReturnOp,
                dialects::qmem::RemoteOp
        >(op);
    }

    std::vector<QuantumOpsGroupTy> functionizeOpClassifier(dialects::qmem::FuncOp &mainFunction) {
        std::vector<QuantumOpsGroupTy> opsGroups;
        std::map<Operation *, std::vector<QuantumOpsGroupTy>> qubitGroupsMap;
        std::set<Operation *> groupedOps;

        LLVM_DEBUG(llvm::dbgs() << "%%%%%%%%%%%%%%%%%%%%%%%%\n");
        LLVM_DEBUG(llvm::dbgs() << "%      CLASSIFIER      %\n");
        LLVM_DEBUG(llvm::dbgs() << "%%%%%%%%%%%%%%%%%%%%%%%%\n");

        /* Usual lifecycle of a qubit:
         * QAlloc -> QInit/EPRS_init/EPRS_measure -> Gate(s) -> Measure
         * In this lifecycle, a qubit "is not usable" until it is (eprs) initialized.
         * WARNING - It is possible that there are quite some operations between the qalloc and
         * the init/eprs operation. *This implementation makes that assumption that the qinit/eprs
         * operation can be moved as close as it can to its respective qalloc operation*. This
         * assumption allows us to simplify the analysis.
         */
        auto qAllocOperations = mainFunction.getOps<dialects::qmem::QAllocOp>();
        for (dialects::qmem::QAllocOp qAllocOp : qAllocOperations) {
            // We will setup the "base" groups here, containing the qalloc + init operations

            // Look for the "init" operation of the current qalloc
            for (Operation *user : qAllocOp->getUsers()) {
                if (qMemOpInitsQubit(user)) {
                    // Found: Group both ops in a single group and mark the operations as grouped
                    QuantumOpsGroupTy newGroup{qAllocOp, user};
                    qubitGroupsMap.insert({qAllocOp.getOperation(), {newGroup}});
                    groupedOps.insert({qAllocOp.getOperation(), user});
                    break;
                }
            }
        }

        // Iterate over all the operations of the main function
        for (Operation &op : mainFunction.getOps()) {
            if (!operationBelongsToQMemDialect(op) || qMemOpShouldRemainInBody(op) || groupedOps.contains(&op)) {
                // Operation is not QMem or it was already grouped
                continue;
            }
            if (qMemOpIsClassicalCommunication(op)) {
                // This operation should stay in the main body...
                // Additionally, this op acts as a "barrier", so we start a new, empty group
                // for each of the involved qubits
                for (auto &qubitGroupsEntry : qubitGroupsMap) {
                    // IMPORTANT: We need to get *a reference* of the group entry.
                    // If we don't declare the entry as a reference, then *it gets copied* to the
                    // local variable, and the newly-created empty group will not be present in the
                    // original map
                    qubitGroupsEntry.second.emplace_back();
                }
                continue;
            }
            // We get the last operations group for the involved qubit
            auto qubitOp = dyn_cast<qoala::helpers::OpQubitsInterface>(op);
            Value involvedQubit = qubitOp.getOperationQubits()[0];
            // Similar as before, we need to *get a reference* of the group to insert the operation
            // If we don't declare the group as a reference, then *it gets copied* into the local variable
            // so the operation will not be inserted in the respective group
            QuantumOpsGroupTy &currentOpsGroup = *qubitGroupsMap[involvedQubit.getDefiningOp()].rbegin();
            // We insert the operation in the group
            currentOpsGroup.push_back(&op);
        }

        // After iterating all the instructions, commit all the discovered groups
        unsigned int groupNum = 0;
        for (auto qubitGroupsEntry : qubitGroupsMap) {
            for (QuantumOpsGroupTy operationsGroup : qubitGroupsEntry.second) {
                if (!operationsGroup.empty()) {
                    LLVM_DEBUG(llvm::dbgs() << " - Discovered Group #" << groupNum++ << " :\n");
                    for (Operation *operation : operationsGroup) {
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
}
