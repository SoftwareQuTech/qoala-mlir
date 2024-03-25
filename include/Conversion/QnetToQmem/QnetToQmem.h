#ifndef QNET_TO_QMEM_H
#define QNET_TO_QMEM_H
#include "mlir/Pass/Pass.h"

// In the Lowering pass, we rely on both HIR and LIR dialects
#include "Dialect/Qnet/QnetDialect.h"
#include "Dialect/lir/LirDialect.h"
#include "Dialect/Qnet/Qnet.h"
#include "Dialect/lir/Lir.h"

namespace mlir {
#define GEN_PASS_DECL
#include "Conversion/QnetToQmem/QnetToQmem.h.inc"

std::unique_ptr<mlir::Pass> createQnetToQmemPass();

/// Generate the code for registering passes.
#define GEN_PASS_REGISTRATION
#include "Conversion/QnetToQmem/QnetToQmem.h.inc"

} // namespace mlir

#endif //QNET_TO_QMEM_H
