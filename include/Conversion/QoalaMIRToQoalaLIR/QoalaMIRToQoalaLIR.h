#ifndef QOALAMIR_TO_QOALALIR_H
#define QOALAMIR_TO_QOALALIR_H
#include "mlir/Pass/Pass.h"

// In the Lowering pass, we rely on both QMem and QoalaHost dialects
#include "Dialect/QMem/QMem.h"
#include "Dialect/QMem/QMemDialect.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Dialect/QoalaHost/QoalaHostDialect.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "Dialect/NetQASM/NetQASMDialect.h"

namespace mlir {
#define GEN_PASS_DECL
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h.inc"

    std::unique_ptr<mlir::Pass> createQoalaMIRToQoalaLIRPass();
    std::unique_ptr<mlir::Pass> createLowerQMemToQoalaHostPass();
    std::unique_ptr<mlir::Pass> createLowerQMemToNetQASMPass();

/// Generate the code for registering passes.
#define GEN_PASS_REGISTRATION
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h.inc"

} // namespace mlir

#endif // QOALAMIR_TO_QOALALIR_H
