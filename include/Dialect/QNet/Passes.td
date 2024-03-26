#ifndef QNET_PASSES
#define QNET_PASSES

include "mlir/Pass/PassBase.td"

def QNetCheckLinear : Pass<"qnet-check-linear"> {
  let summary = "Check if quantum values are used at most once";
  let constructor = "mlir::createQNetCheckLinearPass()";
  let dependentDialects = ["::qoala::dialects::qnet::QNetDialect"];
}

#endif // QNET_PASSES