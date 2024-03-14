#include "lowering/HIRToLIR.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"

namespace mlir {
#define GEN_PASS_DEF_HIRTOLIR
#include "lowering/HIRToLIR.h.inc"
}

using namespace mlir;

class HIRtoLIRPass : public impl::HIRtoLIRBase<HIRtoLIRPass> {
public:
    void runOnOperation() override;
};

void HIRtoLIRPass::runOnOperation()  {
    // TODO
    ModuleOp module = mlir::OperationPass<ModuleOp>::getOperation();
}

std::unique_ptr<mlir::Pass> mlir::createHirToLirPass() {
    return std::make_unique<HIRtoLIRPass>();
}