#ifndef QOALAHOST_PASSES
#define QOALAHOST_PASSES

include "mlir/Pass/PassBase.td"

def QoalaHostAddBlockPrecedences : Pass<"qoalahost-add-block-precedences", "mlir::ModuleOp"> {
    let summary = "Add precedences in QoalaHost blocks definitions. This is meant to be used in the conversion from MIR to LIR and not as stand alone.";
    let dependentDialects = ["qoala::dialects::qoalahost::QoalaHostDialect", "mlir::func::FuncDialect"];
}
def QoalaHostReorderBlocks : Pass<"qoalahost-reorder-blocks", "mlir::ModuleOp"> {
    let summary = "Reorder blocks in QoalaHost main function for minimizing qubits lifetimes.";
    let options = [
        Option<"withDeadlines", "with-deadlines", "bool", /*default=*/"false",
               "Whether to compute and add deadlines when reordering blocks.">,
    ];
    let dependentDialects = ["qoala::dialects::qoalahost::QoalaHostDialect", "mlir::func::FuncDialect"];
}

def QoalaHostQMemEffShowAnalysis: Pass<"qoalahost-show-analysis-qmem-eff", "qoala::dialects::qoalahost::MainFuncOp"> {
    let summary = "Show the quantum memory efficiency analysis computed in LIR.";
    let dependentDialects = ["qoala::dialects::qoalahost::QoalaHostDialect", "mlir::func::FuncDialect"];
}

def QoalaHostQubitLifeShowAnalysis: Pass<"qoalahost-show-analysis-qubit-life", "mlir::ModuleOp"> {
    let summary = "Show the qubit lifetime analysis computed in LIR.";
    let dependentDialects = ["qoala::dialects::qoalahost::QoalaHostDialect", "mlir::func::FuncDialect"];
}

def QoalaHostESPShowAnalysis: Pass<"qoalahost-show-analysis-esp", "mlir::ModuleOp"> {
    let summary = "Show the estimated success probability analysis computed in LIR.";
    let dependentDialects = ["qoala::dialects::qoalahost::QoalaHostDialect", "mlir::func::FuncDialect"];
}

#endif // QOALAHOST_PASSES
