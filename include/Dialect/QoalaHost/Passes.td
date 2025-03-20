#ifndef QOALAHOST_PASSES
#define QOALAHOST_PASSES

include "mlir/Pass/PassBase.td"

def QoalaHostAddBlockDependencies : Pass<"qoalahost-add-block-deps", "mlir::ModuleOp"> {
    let summary = "Add dependencies in Qoala blocks definitions.";
    let dependentDialects = ["::qoala::dialects::qoalahost::QoalaHostDialect", "::mlir::func::FuncDialect"];
}

#endif // QOALAHOST_PASSES
