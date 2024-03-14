#ifndef HIR_TO_LIR_PASS
#define HIR_TO_LIR_PASS

include "mlir/Pass/PassBase.td"

//===----------------------------------------------------------------------===//
// Transformation Pass from HIR to LIR
//===----------------------------------------------------------------------===//

def HIRtoLIR : Pass<"lower-qoala-hir-to-lir", "mlir::ModuleOp"> {
  let summary = "Lower Qoala HIR to LIR";
  let description = [{
    This pass lowers all the instances of the Qoala HIR dialect to the LIR dialect.
  }];
  let constructor = "::mlir::createHirToLirPass()";
  //let dependentDialects = ["::mlir::lir::Lir_Dialect"];
}

#endif /* HIR_TO_LIR_PASS */
