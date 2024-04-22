#ifndef QMEM_TO_QOALAHOST_PASS
#define QMEM_TO_QOALAHOST_PASS

include "mlir/Pass/PassBase.td"

//===----------------------------------------------------------------------===//
// Transformation Pass from QMem to QoalaHost
//===----------------------------------------------------------------------===//

def QoalaMIRToQoalaLIR : Pass<"lower-qoala-mir-to-lir", "mlir::ModuleOp"> {
  let summary = "Lower Qoala MIR to Qoala LIR";
  let description = [{
    This pass lowers all the instances of the Qoala QMem dialect to the QoalaHost dialect.
  }];
  let constructor = "::mlir::createQoalaMIRToQoalaLIRPass()";
  // Since we lower QMem to NetQASM and QoalaHost, we need to depend on all those dialects to create
  // valid MLIR
  let dependentDialects = ["::qoala::dialects::qmem::QMemDialect",
                           "::qoala::dialects::qoalahost::QoalaHostDialect",
                           "::qoala::dialects::netqasm::NetQASMDialect"];
}

#endif /* QMEM_TO_QOALAHOST_PASS */
