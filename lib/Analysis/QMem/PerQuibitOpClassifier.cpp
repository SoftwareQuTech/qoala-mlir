#include "Analysis/QMem/Conversion.h"
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
                dialects::qmem::InitOp
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
        std::map<Value, QuantumOpsGroupTy> perQubitGroup;
        QuantumOpsGroupTy currentOpsGroup;
        std::set<Operation *> groupedOps;

        LLVM_DEBUG(llvm::dbgs() << "%%%%%%%%%%%%%%" << "\n");
        LLVM_DEBUG(llvm::dbgs() << "% CLASSIFIER %" << "\n");

        /* Usual lifecycle of a qubit:
         * QAlloc -> QInit/EPRS_init -> Gate(s) -> (EPRS_)Measure
         * In this lifecycle, a qubit "is not usable" until it is (eprs) initialized.
         * WARNING - It is possible that there are quite some operations between the qalloc and
         * the init/eprs operation. *This implementation makes that assumption that the qinit/eprs
         * operation can be moved as close as it can to its respective qalloc operation*. This
         * assumption allows us to simplify the analysis
         */
        auto qAllocOperations = mainFunction.getOps<dialects::qmem::QAllocOp>();
        for (dialects::qmem::QAllocOp qAllocOp : qAllocOperations) {
            // Special case: qalloc operation has a single use, and it is only for eprs_measure
            bool opIsQAllocEPRSMeasurePair = false;
            if (llvm::hasSingleElement(qAllocOp->getUsers())) {
                opIsQAllocEPRSMeasurePair = true;
            }

            // Look for the "init" operation of the current qalloc
            for (Operation *user : qAllocOp->getUsers()) {
                if (opIsQAllocEPRSMeasurePair || qMemOpInitsQubit(user)) {
                    // Found: Group both ops in a single group and mark the operations as grouped
                    LLVM_DEBUG(llvm::dbgs() << " - op: " << qAllocOp << "\n");
                    LLVM_DEBUG(llvm::dbgs() << " - op: " << *user << "\n");
                    LLVM_DEBUG(llvm::dbgs() << " - New Group" << "\n");
                    opsGroups.push_back({qAllocOp, user});
                    groupedOps.insert(qAllocOp.getOperation());
                    groupedOps.insert(user);
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
                // This function should stay in the main body... commit the current state of the group iff it's not empty
                if (!currentOpsGroup.empty()) {
                    opsGroups.push_back(currentOpsGroup);
                    currentOpsGroup.clear();
                    LLVM_DEBUG(llvm::dbgs() << " - New Group" << "\n");
                }
                continue;
            }

            currentOpsGroup.push_back(&op);
            LLVM_DEBUG(llvm::dbgs() << " - op: " << op << "\n");
        }
        LLVM_DEBUG(llvm::dbgs() << "%%%%%%%%%%%%%%" << "\n");
        return opsGroups;
    }
}
