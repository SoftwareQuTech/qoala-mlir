#ifndef QMEM_PASSES
#define QMEM_PASSES

include "mlir/Pass/PassBase.td"

def QMemFactorizeRemoteDecls : Pass<"qmem-factorize-remotes", "qoala::dialects::qmem::RemoteOp"> {
  let summary = "Factorize all remote declarations by declaring a remote only once and placing them at the module level";
  let constructor = "mlir::createQMemFactorizeRemoteDeclsPass()";
  let dependentDialects = ["::qoala::dialects::qmem::QMemDialect"];
}

#endif // QMEM_PASSES