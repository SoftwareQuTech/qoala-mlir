#ifndef CONVERSION_LIRCOMMONTOLIR_PASSES_TD
#define CONVERSION_LIRCOMMONTOLIR_PASSES_TD

include "mlir/Pass/PassBase.td"

def LirCommonToLirPass : Pass<"convert-lircommon-to-lir", "ModuleOp"> {
  let summary = "Convert lir-common dialect to lir dialect";
  let constructor = "mlir::createLirCommonToLirPass()";
  let dependentDialects = ["hir::HirDialect",
                           "clir::CLirDialect",
                           "qlir::QLirDialect",
                           "lircommon::LirCommonDialect",
                           "StandardOpsDialect",
                           "AffineDialect",
                           "scf::SCFDialect"];
}

def SplitFuncPass : Pass<"split-lircommon-funcs", "FuncOp"> {
  let summary = "Split lir-common operands in different functions";
  let constructor = "mlir::createSplitFuncPass()";
  let dependentDialects = ["hir::HirDialect",
                           "clir::CLirDialect",
                           "qlir::QLirDialect",
                           "lircommon::LirCommonDialect",
                           "StandardOpsDialect"];
}

#endif // CONVERSION_LIRCOMMONTOLIR_PASSES_TD
