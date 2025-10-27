#ifndef QNET_PASSES_H_
#define QNET_PASSES_H_

#include "Dialect/QNet/QNetDialect.h"
#include "mlir/Pass/Pass.h"

namespace qoala::analysis {
#define GEN_PASS_DECL
#include "Dialect/QNet/Passes.h.inc"
    /// Generate the code for registering passes.

#define GEN_PASS_REGISTRATION
#include "Dialect/QNet/Passes.h.inc"

} // namespace qoala::analysis

#endif // QNET_PASSES_H_
