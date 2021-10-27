#ifndef LIR_PASSES
#define LIR_PASSES

include "mlir/Pass/PassBase.td"

def LirRewritePass : FunctionPass<"lir-apply-reordering"> {
    let constructor = "mlir::createLirRewritePass()";
}

#endif // LIR_PASSES