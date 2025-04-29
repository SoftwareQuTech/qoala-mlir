#ifndef QOALAHOST_PASSES
#define QOALAHOST_PASSES

include "mlir/Pass/PassBase.td"

def QoalaHostAddBlockDependencies : Pass<"qoalahost-add-block-deps", "mlir::ModuleOp"> {
    let summary = "Add dependencies in QoalaHost blocks definitions. This is meant to be used in the conversion from MIR to LIR and not as stand alone.";
    let dependentDialects = ["::qoala::dialects::qoalahost::QoalaHostDialect", "::mlir::func::FuncDialect"];
}

#endif // QOALAHOST_PASSES
