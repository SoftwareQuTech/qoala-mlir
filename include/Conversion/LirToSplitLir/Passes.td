#ifndef CONVERSION_LIRTOSPLITLIR_PASSES_TD
#define CONVERSION_LIRTOSPLITLIR_PASSES_TD

include "mlir/Pass/PassBase.td"

def LirToSplitLirPass : Pass<"convert-lir-to-split-lir", "ModuleOp"> {
  let summary = "Convert lir dialect to c-lir and q-lir dialects";
  let constructor = "mlir::createLirToSplitLirPass()";
  let dependentDialects = ["hir::HirDialect",
                           "clir::CLirDialect",
                           "qlir::QLirDialect",
                           "lir::LirDialect",
                           "StandardOpsDialect",
                           "AffineDialect",
                           "scf::SCFDialect"];
}

def SplitFuncPass : Pass<"split-lir-funcs", "FuncOp"> {
  let summary = "Split lir operands in different functions";
  let constructor = "mlir::createSplitFuncPass()";
  let dependentDialects = ["hir::HirDialect",
                           "clir::CLirDialect",
                           "qlir::QLirDialect",
                           "lir::LirDialect",
                           "StandardOpsDialect"];
}

#endif // CONVERSION_LIRTOSPLITLIR_PASSES_TD
