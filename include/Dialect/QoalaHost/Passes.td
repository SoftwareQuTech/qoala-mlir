#ifndef QOALAHOST_PASSES
#define QOALAHOST_PASSES

include "mlir/Pass/PassBase.td"

def QoalaHostAddBlockPrecedences : Pass<"qoalahost-add-block-precedences", "mlir::ModuleOp"> {
    let summary = "Add precedences in QoalaHost blocks definitions. This is meant to be used in the conversion from MIR to LIR and not as stand alone.";
    let dependentDialects = ["::qoala::dialects::qoalahost::QoalaHostDialect", "::mlir::func::FuncDialect"];
}
def QoalaHostReorderBlocks : Pass<"qoalahost-reorder-blocks", "mlir::ModuleOp"> {
    let summary = "Reorder blocks in QoalaHost main function for minimizing qubits lifetimes.";
    let options = [
        Option<"withDeadlines", "with-deadlines", "bool", /*default=*/"false",
               "Whether to compute and add deadlines when reordering blocks.">,
    ];
    let dependentDialects = ["::qoala::dialects::qoalahost::QoalaHostDialect", "::mlir::func::FuncDialect"];
}

def QoalaHostShowAnalysis : Pass<"qoalahost-show-analysis", "mlir::ModuleOp"> {
  let summary = "Runs and shows the results of specified analyses";

  let options = [
    Option<"showQMemEff", "show-qmem-eff", "bool", /*default=*/"false",
           "Run and show QMem Efficiency Analysis.">,
    Option<"showQubitLife", "show-qubit-life", "bool", /*default=*/"false",
           "Run and show Qubit Lifetime Analysis.">
  ];
  let dependentDialects = ["::qoala::dialects::qoalahost::QoalaHostDialect", "::mlir::func::FuncDialect"];
}

#endif // QOALAHOST_PASSES
