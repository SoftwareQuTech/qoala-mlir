#ifndef QNET_PASSES
#define QNET_PASSES

include "mlir/Pass/PassBase.td"

def QnetCheckLinear : Pass<"qnet-check-linear"> {
  let summary = "Check if quantum values are used at most once";
  let constructor = "mlir::createQnetCheckLinearPass()";
  let dependentDialects = ["::qoala::dialects::qnet::QnetDialect"];
}

#endif // QNET_PASSES