#ifndef QNET_TO_QMEM_PASS
#define QNET_TO_QMEM_PASS

include "mlir/Pass/PassBase.td"

//===----------------------------------------------------------------------===//
// Transformation Pass from QNET to LIR
//===----------------------------------------------------------------------===//

def QnetToQmem : Pass<"convert-qnet-to-qmem", "mlir::ModuleOp"> {
  let summary = "Lower Qoala Qnet to Qmem";
  let description = [{
    This pass lowers all the instances of the Qoala Qnet dialect to the Qmem dialect.
  }];
  let constructor = "::mlir::createQnetToQmemPass()";
  // Since we lower Qnet to LIR, we need to depend on both dialects to create
  // valid MLIR
  let dependentDialects = ["::qoala::dialects::qnet::QnetDialect", "lir::LirDialect"];
}

#endif /* QNET_TO_QMEM_PASS */
