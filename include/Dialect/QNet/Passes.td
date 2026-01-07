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
  let options = [
        Option<"withClassicalAwareness", "with-classical-awareness", "bool", /*default=*/"false",
               "Only treat measurements whose classical results are observably used as liveness roots.">,
    ];
}

def QNetGateCountShowAnalysis: Pass<"qnet-show-analysis-gate-count", "mlir::ModuleOp"> {
    let summary = "Show the gate count analysis computed in HIR.";
    let dependentDialects = ["qoala::dialects::qnet::QNetDialect"];
}

def QNetPeepholeOptimizations : Pass<"qnet-peephole-optimizations", "qoala::dialects::qnet::FuncOp"> {
  let summary = "Run peephole optimizations (e.g. hermitian cancellation, rotation merging) on quantum code.";
  let dependentDialects = ["mlir::arith::ArithDialect"];
  let options = [
        Option<"hermitianCancel", "hermitian-cancel", "bool", /*default=*/"true",
               "Run hermitian cancellations as part of the peephole optimizations.">,
        Option<"rotationFolding", "rotation-folding", "bool", /*default=*/"true",
               "Apply rotation merges as part of the peephole optimizations.">,
        Option<"pauliGatesToRotations", "pauli-to-rotations", "bool", /*default=*/"false",
               "Convert Pauli gates to their rotation equivalence.">,
        Option<"twoPiEpsilon", "two-pi-epsilon", "double", /*default=*/"1e-6",
           "Tolerance for recognizing rotation angles equivalent to multiples of 2*pi.">,
    ];
}

#endif // QNET_PASSES