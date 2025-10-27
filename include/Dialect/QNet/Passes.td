#ifndef QNET_PASSES
#define QNET_PASSES

include "mlir/Pass/PassBase.td"

def QNetCheckLinear : Pass<"qnet-check-linear"> {
  let summary = "Check if quantum values are used at most once.";
  let dependentDialects = ["qoala::dialects::qnet::QNetDialect"];
}

def QNetDeadCodeElimination : Pass<"qnet-dead-code-elimination", "qoala::dialects::qnet::FuncOp"> {
  let summary = "Run Dead Code Elimination (DCE) on quantum code.";
  let dependentDialects = ["qoala::dialects::qnet::QNetDialect"];
}

#endif // QNET_PASSES