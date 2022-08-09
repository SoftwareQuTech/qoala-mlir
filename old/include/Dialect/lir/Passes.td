#ifndef LIR_PASSES
#define LIR_PASSES

include "mlir/Pass/PassBase.td"

def LirReorderDownPass : FunctionPass<"lir-apply-reordering-down"> {
    let constructor = "mlir::createLirReorderDownPass()";
}

def LirReorderUpPass : FunctionPass<"lir-apply-reordering-up"> {
    let constructor = "mlir::createLirReorderUpPass()";
}

def LirGatePass : FunctionPass<"lir-optimize-gates"> {
    let constructor = "mlir::createLirGatePass()";
}

#endif // LIR_PASSES