#ifndef QNET_PASSES_H_
#define QNET_PASSES_H_

#include "Dialect/Qnet/QnetDialect.h"
#include "mlir/Pass/Pass.h"

namespace mlir {
#define GEN_PASS_DECL
#include "Dialect/Qnet/Passes.h.inc"

std::unique_ptr<Pass> createQnetCheckLinearPass();

/// Generate the code for registering passes.
#define GEN_PASS_REGISTRATION
#include "Dialect/Qnet/Passes.h.inc"

} // namespace mlir

#endif // QNET_PASSES_H_