#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"

#include "Dialect/qoala_lir_m/QoalaLirM.h"
#include "Dialect/qoala_lir_m/Transforms/QoalaLirMTransforms.h"

using namespace mlir;
using namespace mlir::qoala_lir_m;

struct LirMGenericPass : public PassWrapper<LirMGenericPass, OperationPass<NetqasmFuncOp>>
{
    void runOnOperation() override
    {
        Operation *op = getOperation();
        llvm::outs() << op << "\n";
    }
};