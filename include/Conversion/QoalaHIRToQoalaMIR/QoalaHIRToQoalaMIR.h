#ifndef QNET_TO_QMEM_H
#define QNET_TO_QMEM_H
#include "mlir/Pass/Pass.h"

// In the Lowering pass, we rely on both QNet and QMem dialects
#include "Dialect/QMem/QMem.h"
#include "Dialect/QMem/QMemDialect.h"
#include "Dialect/QNet/QNet.h"
#include "Dialect/QNet/QNetDialect.h"

namespace qoala::conversion {
#define GEN_PASS_DECL
#include "Conversion/QoalaHIRToQoalaMIR/QoalaHIRToQoalaMIR.h.inc"

/// Generate the code for registering passes.
#define GEN_PASS_REGISTRATION
#include "Conversion/QoalaHIRToQoalaMIR/QoalaHIRToQoalaMIR.h.inc"

} // namespace qoala::conversion

#endif // QNET_TO_QMEM_H
