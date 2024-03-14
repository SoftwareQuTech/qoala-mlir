#ifndef QOALA_MLIR_HIRTOLIR_H
#define QOALA_MLIR_HIRTOLIR_H

#include "Dialect/hir/HirDialect.h"
#include "mlir/Pass/Pass.h"

namespace mlir {
#define GEN_PASS_DECL
#include "lowering/HIRToLIR.h.inc"

std::unique_ptr<mlir::Pass> createHirToLirPass();

/// Generate the code for registering passes.
#define GEN_PASS_REGISTRATION
#include "lowering/HIRToLIR.h.inc"

} // namespace mlir

#endif //QOALA_MLIR_HIRTOLIR_H
