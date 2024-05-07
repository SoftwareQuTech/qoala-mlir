#ifndef QMEM_PASSES_H_
#define QMEM_PASSES_H_

#include "Dialect/QMem/QMemDialect.h"
#include "mlir/Pass/Pass.h"

namespace mlir {
#define GEN_PASS_DECL
#include "Dialect/QMem/Passes.h.inc"

std::unique_ptr<Pass> createQMemSimpleFunctionize();
std::unique_ptr<Pass> createQMemF32RotationsConversion();

/// Generate the code for registering passes.
#define GEN_PASS_REGISTRATION
#include "Dialect/QMem/Passes.h.inc"

} // namespace mlir

#endif // QMEM_PASSES_H_