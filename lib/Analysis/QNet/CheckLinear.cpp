#include "Dialect/QNet/Passes.h"
#include "Dialect/QNet/QNet.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "llvm/Support/Debug.h"

namespace mlir {
#define GEN_PASS_DEF_QNETCHECKLINEAR
#include "Dialect/QNet/Passes.h.inc"
} // namespace mlir

using namespace mlir;
using namespace qoala::dialects::qnet;

namespace {

struct QNetCheckLinearPass
    : public impl::QNetCheckLinearBase<QNetCheckLinearPass> {
    void runOnOperation() override;
};

} // namespace

bool isQuantumOp(mlir::Operation *op) {
    return llvm::isa<NewQubitOp, RotXOp, RotYOp, RotZOp, HadamardOp, CnotOp, CzOp,
                     CrotXOp, MeasureOp, EprsOp, EprsMeasureOp>(op);
}

void QNetCheckLinearPass::runOnOperation() {
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
}

std::unique_ptr<Pass> mlir::createQNetCheckLinearPass() {
    return std::make_unique<QNetCheckLinearPass>();
}
