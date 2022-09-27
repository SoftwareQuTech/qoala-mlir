#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"

#include "Dialect/qoala_lir_m/QoalaLirM.h"
#include "Dialect/qoala_lir_m/Transforms/QoalaLirMTransforms.h"

using namespace mlir;
using namespace mlir::qoala_lir_m;

class LirMGenericPass : public PassWrapper<LirMGenericPass, OperationPass<NetqasmFuncOp>>
// class LirMGenericPass : public PassWrapper<LirMGenericPass, OperationPass<>>
{
    void runOnOperation() override
    {
        Operation *op = getOperation();
        NetqasmFuncOp funcOp = llvm::dyn_cast<NetqasmFuncOp>(getOperation());
        llvm::outs() << op->getName() << "\n";
        llvm::outs() << funcOp.getSymName() << "\n";

        Region &body = funcOp.getBody();
        for (auto &body_op : body.getOps())
        {
            llvm::outs() << body_op.getName() << "\n";
        }
    }

    StringRef getArgument() const final
    {
        return "generic-pass";
    }
};

std::unique_ptr<mlir::Pass> createLirMGenericPass()
{
    return std::make_unique<LirMGenericPass>();
}