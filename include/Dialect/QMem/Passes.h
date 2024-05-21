#ifndef QMEM_PASSES_H_
#define QMEM_PASSES_H_

#include "Dialect/QMem/QMemDialect.h"
#include "mlir/Pass/Pass.h"

namespace qoala::analysis {
#define GEN_PASS_DECL
#include "Dialect/QMem/Passes.h.inc"

/// Generate the code for registering passes.
#define GEN_PASS_REGISTRATION
#include "Dialect/QMem/Passes.h.inc"

} // namespace mlir

#endif // QMEM_PASSES_H_