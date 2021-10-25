#ifndef LIRCOMMON_PASSES
#define LIRCOMMON_PASSES

include "mlir/Pass/PassBase.td"

def LirCommonRewritePass : FunctionPass<"lir-apply-reordering"> {
    let constructor = "mlir::createLirCommonRewritePass()";
}

#endif // LIRCOMMON_PASSES