#ifndef QMEM_PASSES
#define QMEM_PASSES

include "mlir/Pass/PassBase.td"

def QMemSimpleFunctionize : Pass<"qmem-simple-functionize", "mlir::ModuleOp"> {
  let summary = "PoC; DO NOT USE - Extracts each QMem quantum operation into a single func.func function";
  let constructor = "mlir::createQMemSimpleFunctionize()";
  let dependentDialects = ["::qoala::dialects::qmem::QMemDialect", "::mlir::func::FuncDialect"];
}

#endif // QMEM_PASSES