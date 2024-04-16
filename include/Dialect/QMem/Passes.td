#ifndef QMEM_PASSES
#define QMEM_PASSES

include "mlir/Pass/PassBase.td"

def QMemFactorizeRemoteDecls : Pass<"qmem-factorize-remotes", "mlir::ModuleOp"> {
  let summary = "Factorize all remote declarations by declaring a remote only once and placing them at the module level";
  let constructor = "mlir::createQMemFactorizeRemoteDeclsPass()";
  let dependentDialects = ["::qoala::dialects::qmem::QMemDialect"];
}

def QMemSimpleFunctionize : Pass<"qmem-simple-functionize", "mlir::ModuleOp"> {
  let summary = "PoC; DO NOT USE - Extracts each QMem quantum operation into a single function";
  let constructor = "mlir::createQMemSimpleFunctionize()";
  let dependentDialects = ["::qoala::dialects::qmem::QMemDialect", "::mlir::func::FuncDialect"];
}

#endif // QMEM_PASSES