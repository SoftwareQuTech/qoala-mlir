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

    static inline bool isCandidateForErasure(Operation *op) {
        return isa<NewQubitOp, RotXOp, RotYOp, RotZOp, HadamardOp, CnotOp, CzOp, CrotXOp>(op);
    }

    void QNetDeadCodeEliminationPass::runOnOperation() {
        LLVM_DEBUG(llvm::dbgs() << "[QNet][DCE] starts...\n");

        Operation *operation = getOperation();
        ModuleOp moduleOp = llvm::dyn_cast<ModuleOp>(operation);
        const auto funcs = moduleOp.getOps<FuncOp>();
        assert(!funcs.empty() && "No func? This is embarrassing...");
        FuncOp mainFunc = *funcs.begin();

        std::vector<Operation *> worklist;
        DenseSet<Operation *> live;

        mainFunc.walk([&](Operation *op) {
            if (isa<MeasureOp>(op)) {
                worklist.push_back(op);
            }
            if (isa<EprsOp>(op) || isa<EprsMeasureOp>(op)) {
                live.insert(op);
            }
        });

        while (!worklist.empty()) {
            Operation *op = worklist.back();
            worklist.pop_back();

            if (live.find(op) == live.end()) {
                live.insert(op);
                for (Value operand : op->getOperands()) {
                    Operation *producer = operand.getDefiningOp();
                    if (producer != nullptr) {
                        worklist.push_back(producer);
                    }
                }
            }
        }

        std::vector<Operation *> toErase;
        mainFunc.walk([&](Operation *op) {
            if (isCandidateForErasure(op)) {
                bool isLive = (live.find(op) != live.end());
                if (!isLive) {
                    toErase.push_back(op);
                }
            };
        });

        for (Operation *op : toErase) {
            if (isa<NewQubitOp>(op)) {
                op->emitWarning("Removed unused locally initialized qubit");
            }
        }

        for (auto it = toErase.rbegin(); it != toErase.rend(); ++it) {
            Operation *op = *it;
            LLVM_DEBUG(llvm::dbgs() << "[QNet][DCE] erasing: " << *op << "\n");
            op->erase();
        }

        LLVM_DEBUG(llvm::dbgs() << "[QNet][DCE] done\n");
    }
} /* namespace qoala::analysis */
