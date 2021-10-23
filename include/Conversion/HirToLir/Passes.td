#ifndef CONVERSION_HIRTOLIR_PASSES_TD
#define CONVERSION_HIRTOLIR_PASSES_TD

include "mlir/Pass/PassBase.td"

def HirToLirPass : Pass<"convert-hir-to-lir", "ModuleOp"> {
  let summary = "Convert hir dialect to lir dialect";
  let constructor = "mlir::createHirToLirPass()";
  let dependentDialects = ["hir::HirDialect",
                           "lir::LirDialect",
                           "StandardOpsDialect",
                           "AffineDialect",
                           "scf::SCFDialect"];
}

#endif // CONVERSION_HIRTOLIR_PASSES_TD
