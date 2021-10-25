#ifndef CONVERSION_HIRTOLIR_PASSES_H
#define CONVERSION_HIRTOLIR_PASSES_H

#include "mlir/Pass/Pass.h"

namespace mlir
{

    class Pass;

    std::unique_ptr<Pass> createHirToLirCommonPass();

#define GEN_PASS_REGISTRATION
#include "Conversion/HirToLirCommon/Passes.h.inc"

} // namespace mlir

#endif // CONVERSION_HIRTOLIR_PASSES_H
