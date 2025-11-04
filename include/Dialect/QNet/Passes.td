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

def QNetPeepholeOptimizations : Pass<"qnet-peephole-optimizations", "qoala::dialects::qnet::FuncOp"> {
  let summary = "Run peephole optimizations (e.g. hermitian cancellation, rotation merging) on quantum code.";
  let options = [
        Option<"hermiatianCancel", "hermitian-cancel", "bool", /*default=*/"true",
               "Run hermitian cancellations as part of the peephole optimizations.">,
    ];
}

#endif // QNET_PASSES