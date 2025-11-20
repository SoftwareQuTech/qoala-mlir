#include "Dialect/QNet/Passes.h"
#include "Dialect/QNet/QNet.h"
#include "llvm/Support/Debug.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"

#define DEBUG_TYPE "qnet-dead-code-elimination-pass"

using namespace mlir;
using namespace qoala::dialects::qnet;

namespace qoala::analysis {
#define GEN_PASS_DEF_QNETDEADCODEELIMINATION
#include "Dialect/QNet/Passes.h.inc"

    class QNetDeadCodeEliminationPass : public impl::QNetDeadCodeEliminationBase<QNetDeadCodeEliminationPass> {
        using QNetDeadCodeEliminationBase::QNetDeadCodeEliminationBase;
        void runOnOperation() override;
    };

    static bool isCandidateForErasure(Operation *op) {
        return isa<NewQubitOp, RotXOp, RotYOp, RotZOp, HadamardOp, CnotOp, CzOp, CrotXOp>(op);
    }

    void QNetDeadCodeEliminationPass::runOnOperation() {
        // This pass performs dead code elimination for the QNet dialect within the `qnet.func`.
        // The goal is to remove quantum operations that have no observable classical effect, according to the quantum
        // SSA/DFG semantics used in QNet.
        // - An operation is considered *live* if it directly or indirectly contributes to a measurement, or to an
        // operation that must be preserved (EPR creation).
        // - All other operations—such as local gates or allocations whose results are never measured—are treated as
        // dead and removed.
        //
        // Notes and assumptions:
        // - There is exactly one `qnet.func` per module.
        // - The dialect currently treats all unitaries as pure (no side effects).
        // - Measurements are always retained in this version; later iterations may refine this to only keep
        // measurements whose classical outcomes are actually consumed or exported (issue #106).
        // - Remote entanglement creation (`qnet.eprs`) is never removed, as doing so could alter global protocol
        // structure.
        // - The pass assumes the IR is already in SSA form and free of structural errors; no CFG analysis is performed
        // here.

        LLVM_DEBUG(llvm::dbgs() << "[QNet][DCE] starts...\n");

        FuncOp mainFunc = getOperation();

        std::vector<Operation *> worklist;
        DenseSet<Operation *> live;

        // Collect initial liveness roots and must-keep operations:
        // - Measurements are always considered live roots.
        // - EPR creation are inserted into the live set unconditionally as they must never be removed.
        mainFunc.walk([&](Operation *op) {
            if (isa<MeasureOp>(op)) {
                worklist.push_back(op);
            }
            if (isa<EprsOp>(op) || isa<EprsMeasureOp>(op)) {
                live.insert(op);
            }
        });

        // Backward liveness propoagation along SSA def-use chains.
        while (!worklist.empty()) {
            Operation *op = worklist.back();
            worklist.pop_back();

            if (!live.contains(op)) {
                live.insert(op);
                for (Value operand : op->getOperands()) {
                    if (Operation *producer = operand.getDefiningOp()) {
                        worklist.push_back(producer);
                    }
                }
            }
        }

        // Identify dead operations.
        std::vector<Operation *> toErase;
        mainFunc.walk([&](Operation *op) {
            if (isCandidateForErasure(op)) {
                if (!live.contains(op)) {
                    toErase.push_back(op);
                }
            }
        });

        // Emit warning for removed local qubit allocations.
        for (Operation *op : toErase) {
            if (isa<NewQubitOp>(op)) {
                op->emitWarning("Removed unused locally initialized qubit");
            }
        }

        // Erase dead operations in **reverse** order
        for (auto it = toErase.rbegin(); it != toErase.rend(); ++it) {
            Operation *op = *it;
            LLVM_DEBUG(llvm::dbgs() << "[QNet][DCE] erasing: " << *op << "\n");
            op->erase();
        }

        LLVM_DEBUG(llvm::dbgs() << "[QNet][DCE] done\n");
    }
} /* namespace qoala::analysis */
