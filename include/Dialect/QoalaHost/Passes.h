#ifndef QOALAHOST_PASSES_H_
#define QOALAHOST_PASSES_H_

#include "Dialect/QoalaHost/QoalaHostDialect.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"

namespace qoala::analysis {
#define GEN_PASS_DECL
#include "Dialect/QoalaHost/Passes.h.inc"

/// Generate the code for registering passes.
#define GEN_PASS_REGISTRATION
#include "Dialect/QoalaHost/Passes.h.inc"

} // namespace mlir

#endif // QOALAHOST_PASSES_H_
