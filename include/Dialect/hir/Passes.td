#ifndef HIR_PASSES
#define HIR_PASSES

include "mlir/Pass/PassBase.td"

def HirRewritePass : Pass<"hir-apply-rewrites"> {
    let constructor = "mlir::createHirRewritePass()";
}

#endif // HIR_PASSES