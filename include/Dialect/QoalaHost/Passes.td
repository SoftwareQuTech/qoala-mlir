#ifndef QOALAHOST_PASSES
#define QOALAHOST_PASSES

include "mlir/Pass/PassBase.td"

def QoalaHostAddBlockPrecedences : Pass<"qoalahost-add-block-precedences", "mlir::ModuleOp"> {
    let summary = "Add precedences in QoalaHost blocks definitions. This is meant to be used in the conversion from MIR to LIR and not as stand alone.";
    let dependentDialects = ["::qoala::dialects::qoalahost::QoalaHostDialect", "::mlir::func::FuncDialect"];
}

def QoalaHostQMemEffShowAnalysis: Pass<"qoalahost-show-analysis-qmem-eff", "mlir::ModuleOp"> {
    let summary = "Show the quantum memory efficiency analysis computed in LIR.";
    let dependentDialects = ["::qoala::dialects::qoalahost::QoalaHostDialect", "::mlir::func::FuncDialect"];
}

#endif // QOALAHOST_PASSES
