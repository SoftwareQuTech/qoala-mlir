#ifndef CONVERSION_LIRCOMMONTOLIR_PASSES_H
#define CONVERSION_LIRCOMMONTOLIR_PASSES_H

#include "mlir/Pass/Pass.h"

namespace mlir
{

    class Pass;

    std::unique_ptr<Pass> createLirCommonToLirPass();
    std::unique_ptr<Pass> createSplitFuncPass();

#define GEN_PASS_REGISTRATION
#include "Conversion/LirCommonToLir/Passes.h.inc"

} // namespace mlir

#endif // CONVERSION_LIRCOMMONTOLIR_PASSES_H
