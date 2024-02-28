#ifndef HIR_PASSES
#define HIR_PASSES

include "mlir/Pass/PassBase.td"

def HirCheckLinear : Pass<"hir-check-linear"> {
  let summary = "Check if quantum values are used at most once";
  let constructor = "mlir::createHirCheckLinearPass()";
  let dependentDialects = ["hir::HirDialect"];
}

#endif // HIR_PASSES