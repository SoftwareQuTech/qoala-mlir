#ifndef QNET_TO_QMEM_PASS
#define QNET_TO_QMEM_PASS

include "mlir/Pass/PassBase.td"

//===----------------------------------------------------------------------===//
// Transformation Pass from QNET to LIR
//===----------------------------------------------------------------------===//

def QNetToQMem : Pass<"convert-qnet-to-qmem", "mlir::ModuleOp"> {
  let summary = "Lower Qoala QNet to QMem";
  let description = [{
    This pass lowers all the instances of the Qoala QNet dialect to the QMem dialect.
  }];
  let constructor = "::mlir::createQNetToQMemPass()";
  // Since we lower QNet to QMem, we need to depend on both dialects to create
  // valid MLIR
  let dependentDialects = ["::qoala::dialects::qnet::QNetDialect", "::qoala::dialects::qmem::QMemDialect"];
}

#endif /* QNET_TO_QMEM_PASS */
