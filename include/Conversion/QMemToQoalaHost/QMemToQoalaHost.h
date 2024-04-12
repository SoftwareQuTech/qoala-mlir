#ifndef QMEM_TO_QOALAHOST_H
#define QMEM_TO_QOALAHOST_H
#include "mlir/Pass/Pass.h"

// In the Lowering pass, we rely on both QMem and QoalaHost dialects
#include "Dialect/QMem/QMem.h"
#include "Dialect/QMem/QMemDialect.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Dialect/QoalaHost/QoalaHostDialect.h"

namespace mlir {
#define GEN_PASS_DECL
#include "Conversion/QMemToQoalaHost/QMemToQoalaHost.h.inc"

std::unique_ptr<mlir::Pass> createQMemToQoalaHostPass();

/// Generate the code for registering passes.
#define GEN_PASS_REGISTRATION
#include "Conversion/QMemToQoalaHost/QMemToQoalaHost.h.inc"

} // namespace mlir

#endif // QMEM_TO_QOALAHOST_H
