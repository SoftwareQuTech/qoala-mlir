#include "Dialect/Qnet/Passes.h"
#include "Dialect/Qnet/Qnet.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "llvm/Support/Debug.h"

namespace mlir {
#define GEN_PASS_DEF_QNETCHECKLINEAR
#include "Dialect/Qnet/Passes.h.inc"
} // namespace mlir

using namespace mlir;
using namespace qoala::dialects::qnet;

namespace {

struct QnetCheckLinearPass
    : public impl::QnetCheckLinearBase<QnetCheckLinearPass> {
    void runOnOperation() override;
};

} // namespace

/*
 * Helper function to quickly check if a mlir::Operation* can be
 * casted into one of the Op classes provided as a list.
 */
template <typename T, typename... Rest> bool tryCast(mlir::Operation *op) {
    if (auto castedOp = llvm::dyn_cast<T>(op)) {
        return true;
    } else if constexpr (sizeof...(Rest) > 0) {
        return tryCast<Rest...>(op);
    }
    return false;
}

bool isQuantumOp(mlir::Operation *op) {
    return tryCast<NewQubitOp, RotXOp, RotYOp, RotZOp, HadamardOp, CnotOp, CzOp,
                   CrotXOp, MeasureOp, EprsOp, EprsMeasureOp>(op);
}

void QnetCheckLinearPass::runOnOperation() {
    // llvm::dbgs() << "\n=== Start of CheckLinear pass ===\n\n";

    Operation *operation = getOperation();

    if (ModuleOp module = llvm::dyn_cast<ModuleOp>(operation)) {
        // Walk over all ops in the module.
        module.walk([this](Operation *op) {
            if (isQuantumOp(op)) {
                // For all ops that produce quantum values, check the results.
                for (auto result : op->getResults()) {
                    // Check if the result value has zero or one use.
                    if (!result.hasOneUse() && !result.use_empty()) {
                        // More than 1 use.
                        for (auto &use : result.getUses()) {
                            auto usingOp = use.getOwner();
                            usingOp->emitError(
                                "quantum operation is used more than once\n");
                        }
                        signalPassFailure();
                    }
                }
            }
        });
    } else {
        operation->emitError("Not a ModuleOp: something went wrong.");
        signalPassFailure();
        return;
    }

    // llvm::dbgs() << "\n=== End of CheckLinear pass ===\n\n";
}

std::unique_ptr<Pass> mlir::createQnetCheckLinearPass() {
    return std::make_unique<QnetCheckLinearPass>();
}
