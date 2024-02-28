#ifndef HIR_PASSES_H_
#define HIR_PASSES_H_

#include "mlir/Pass/Pass.h"
#include "Dialect/hir/HirDialect.h"

namespace mlir {
#define GEN_PASS_DECL
#include "Dialect/hir/Passes.h.inc"

std::unique_ptr<Pass> createHirCheckLinearPass();

/// Generate the code for registering passes.
#define GEN_PASS_REGISTRATION
#include "Dialect/hir/Passes.h.inc"

} // namespace mlir

#endif // HIL_PASSES_H_