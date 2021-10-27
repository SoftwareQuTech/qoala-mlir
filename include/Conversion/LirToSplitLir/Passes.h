#ifndef CONVERSION_LIRTOSPLITLIR_PASSES_H
#define CONVERSION_LIRTOSPLITLIR_PASSES_H

#include "mlir/Pass/Pass.h"

namespace mlir
{

    class Pass;

    std::unique_ptr<Pass> createLirToSplitLirPass();
    std::unique_ptr<Pass> createSplitFuncPass();

#define GEN_PASS_REGISTRATION
#include "Conversion/LirToSplitLir/Passes.h.inc"

} // namespace mlir

#endif // CONVERSION_LIRTOSPLITLIR_PASSES_H
