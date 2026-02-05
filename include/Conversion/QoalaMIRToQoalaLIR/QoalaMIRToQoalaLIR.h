#ifndef QOALAMIR_TO_QOALALIR_H
#define QOALAMIR_TO_QOALALIR_H
#include "mlir/Pass/Pass.h"

// In the Lowering pass, we rely on both QMem and QoalaHost dialects
#include "Dialect/NetQASM/NetQASM.h"
#include "Dialect/NetQASM/NetQASMDialect.h"
#include "Dialect/QMem/QMem.h"
#include "Dialect/QMem/QMemDialect.h"
#include "Dialect/QRemote/QRemote.h"
#include "Dialect/QRemote/QRemoteDialect.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Dialect/QoalaHost/QoalaHostDialect.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlow.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"

namespace qoala::conversion {
#define GEN_PASS_DECL
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h.inc"

/// Generate the code for registering passes.
#define GEN_PASS_REGISTRATION
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h.inc"

} // namespace qoala::conversion

#endif // QOALAMIR_TO_QOALALIR_H
