#ifndef CONVERSION_HIRTOLIR_PASSES_TD
#define CONVERSION_HIRTOLIR_PASSES_TD

include "mlir/Pass/PassBase.td"

def HirToLirCommonPass : Pass<"convert-hir-to-lircommon", "ModuleOp"> {
  let summary = "Convert hir dialect to lir-common dialect";
  let constructor = "mlir::createHirToLirCommonPass()";
  let dependentDialects = ["hir::HirDialect",
                           "clir::CLirDialect",
                           "qlir::QLirDialect",
                           "lircommon::LirCommonDialect",
                           "StandardOpsDialect",
                           "AffineDialect",
                           "scf::SCFDialect"];
}

#endif // CONVERSION_HIRTOLIR_PASSES_TD
