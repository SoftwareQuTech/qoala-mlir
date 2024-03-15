#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "llvm/Support/Debug.h"

#include "lowering/HIRToLIR.h"
#include "lowering/Helpers.h"

namespace mlir {
#define GEN_PASS_DEF_HIRTOLIR
#include "lowering/HIRToLIR.h.inc"
}

using namespace mlir;
using namespace llvm;
using namespace qoala::helpers;

class HIRtoLIRPass : public mlir::impl::HIRtoLIRBase<HIRtoLIRPass> {
public:
    void runOnOperation() override;

};

void HIRtoLIRPass::runOnOperation()  {
    Operation *op = getOperation();
    qoala::helpers::resetIndent();
    qoala::helpers::printOperation(op);
}

std::unique_ptr<mlir::Pass> mlir::createHirToLirPass() {
    return std::make_unique<HIRtoLIRPass>();
}