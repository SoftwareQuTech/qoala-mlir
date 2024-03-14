#include "Dialect/hir/Passes.h"
#include "lowering/HIRToLIR.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"

namespace mlir {
#define GEN_PASS_DEF_HIRTOLIR
#include "lowering/HIRToLIR.h.inc"
}

class HIRtoLIRPass : public mlir::impl::HIRtoLIRBase<HIRtoLIRPass> {
public:
    void runOnOperation() override {
        // TODO
    }
};

std::unique_ptr<mlir::Pass> mlir::createHirToLirPass() {
    return std::make_unique<HIRtoLIRPass>();
}