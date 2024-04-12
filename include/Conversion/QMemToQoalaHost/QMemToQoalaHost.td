#ifndef QMEM_TO_QOALAHOST_PASS
#define QMEM_TO_QOALAHOST_PASS

include "mlir/Pass/PassBase.td"

//===----------------------------------------------------------------------===//
// Transformation Pass from QMem to QoalaHost
//===----------------------------------------------------------------------===//

def QMemtoQoalaHost : Pass<"convert-qmem-to-qoalahost", "mlir::ModuleOp"> {
  let summary = "Lower Qoala QMem to Qoala Host";
  let description = [{
    This pass lowers all the instances of the Qoala QMem dialect to the QoalaHost dialect.
  }];
  let constructor = "::mlir::createQMemToQoalaHostPass()";
  // Since we lower QMem to QoalaHost, we need to depend on both dialects to create
  // valid MLIR
  let dependentDialects = ["::qoala::dialects::qmem::QMemDialect", "::qoala::dialects::qoalahost::QoalaHostDialect"];
}

#endif /* QMEM_TO_QOALAHOST_PASS */
