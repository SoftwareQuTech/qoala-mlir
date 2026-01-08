#include "Dialect/QNet/Passes.h"
#include "Dialect/QNet/QNet.h"
#include "llvm/Support/Debug.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlow.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

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
        return isa<NewQubitOp, RotXOp, RotYOp, RotZOp, HadamardOp, XOp, YOp, ZOp, CnotOp, CzOp, CrotXOp>(op);
    }

    static bool isQNetObservableClassicalUse(OpOperand &use) {
        Operation *user = use.getOwner();
        Value usedVal = use.get();

        // qnet.return: returning the measurement result to the caller is observable.
        if (isa<ReturnOp>(user)) {
            return true;
        }

        // qnet.send_*: exporting the value to a remote node is observable.
        // Only treat the 'cin' operand as the exported payload.
        if (auto send = dyn_cast<SendIntOp>(user)) {
            return usedVal == send.getCin();
        }
        if (auto send = dyn_cast<SendFloatOp>(user)) {
            return usedVal == send.getCin();
        }
        if (auto send = dyn_cast<SendIntsOp>(user)) {
            return usedVal == send.getCin();
        }
        if (auto send = dyn_cast<SendFloatsOp>(user)) {
            return usedVal == send.getCin();
        }

        return false;
    }

    static bool isObservableClassicalUse(OpOperand &use) {
        Operation *user = use.getOwner();
        Value usedVal = use.get();

        // First handle QNet-specific sinks.
        if (isQNetObservableClassicalUse(use)) {
            return true;
        }

        // Function return.
        if (isa<func::ReturnOp>(user)) {
            return true;
        }

        // Classical CFG branching.
        if (auto condBr = dyn_cast<cf::CondBranchOp>(user)) {
            return usedVal == condBr.getCondition();
        }

        if (auto sw = dyn_cast<cf::SwitchOp>(user)) {
            return usedVal == sw.getFlag();
        }

        // Structured control flow.
        if (auto ifOp = dyn_cast<scf::IfOp>(user)) {
            return usedVal == ifOp.getCondition();
        }

        // Stores are observable effects (value escapes).
        // If we want to be stricter, we can only treat the *stored value* operand as observable.
        // But we are not using this op at the moment anyway...
        if (auto store = dyn_cast<memref::StoreOp>(user)) {
            return usedVal == store.getValueToStore();
        }

        // Calls / ops with side effects: if the measurement-derived value is used by an op that
        // may have effects, treat it as observable.
        // This is conservative, but matches "externally visible effects" well.
        if (auto iface = dyn_cast<mlir::MemoryEffectOpInterface>(user)) {
            if (!iface.hasNoEffect()) {
                return true;
            }
        }
        return false;
    }

    static bool measurementResultIsObserved(MeasureOp measure) {
        // A MeasureOp might produce one classical result.
        // We treat it as observed if the result reaches an observable sink.
        Value outcome = measure.getOutcome();

        SmallVector<Value> worklist;
        worklist.push_back(outcome);

        DenseSet<Value> visited;

        while (!worklist.empty()) {
            Value v = worklist.pop_back_val();
            if (!visited.insert(v).second) {
                continue;
            }

            for (OpOperand &use : v.getUses()) {
                // If v reaches an observable sink, the measurement is observed.
                if (isObservableClassicalUse(use)) {
                    return true;
                }

                Operation *user = use.getOwner();

                // Don't propagate through terminators (except ReturnOp which we already handled).
                if (user->hasTrait<mlir::OpTrait::IsTerminator>()) {
                    continue;
                }

                // Propagate influence forward: any result of an op using v may carry the value onward.
                for (Value res : user->getResults()) {
                    worklist.push_back(res);
                }
            }
        }

        return false;
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

        LLVM_DEBUG(llvm::dbgs() << "[QNet][DCE] starts, with classical awareness=" << this->withClassicalAwareness
                                << "\n");

        FuncOp mainFunc = getOperation();

        std::vector<Operation *> worklist;
        DenseSet<Operation *> live;

        // Collect initial liveness roots and must-keep operations:
        // - By default, all measurements are treated as liveness roots.
        // - When `withClassicalAwareness` is enabled, a measurement is a liveness root only if its
        //   classical outcome is observably consumed (e.g. returned or sent to a remote node).
        // - EPR creation operations are inserted into the live set unconditionally, as they must
        //   never be removed.
        mainFunc.walk([&](Operation *op) {
            if (auto meas = dyn_cast<MeasureOp>(op)) {
                if (!withClassicalAwareness) {
                    // Default behavior: all measurements are roots.
                    worklist.push_back(op);
                } else {
                    // Refined behavior: only measurements with observed classical results are roots.
                    if (measurementResultIsObserved(meas)) {
                        worklist.push_back(op);
                    }
                }
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
            // In refined mode, dead measurements can be erased too.
            if (withClassicalAwareness && isa<MeasureOp>(op)) {
                if (!live.contains(op)) {
                    toErase.push_back(op);
                }
                return;
            }

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
