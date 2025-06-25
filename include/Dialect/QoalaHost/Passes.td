#ifndef QOALAHOST_PASSES
#define QOALAHOST_PASSES

include "mlir/Pass/PassBase.td"

def QoalaHostAddBlockPrecedences : Pass<"qoalahost-add-block-precedences", "mlir::ModuleOp"> {
    let summary = "Add precedences in QoalaHost blocks definitions. This is meant to be used in the conversion from MIR to LIR and not as stand alone.";
    let dependentDialects = ["::qoala::dialects::qoalahost::QoalaHostDialect", "::mlir::func::FuncDialect"];
}

def QoalaHostQMemoryUsage : Pass<"qoalahost-qmem-eff", "mlir::ModuleOp"> {
    let summary = "Compute quantum memory usage in LIR.";
    let dependentDialects = ["::qoala::dialects::qoalahost::QoalaHostDialect", "::mlir::func::FuncDialect"];
}

def QoalaHostShowAnalysis: Pass<"qoalahost-show-analysis", "mlir::ModuleOp"> {
    let summary = "Show the different analysis computed in LIR.";
    let dependentDialects = ["::qoala::dialects::qoalahost::QoalaHostDialect", "::mlir::func::FuncDialect"];
}

#endif // QOALAHOST_PASSES
