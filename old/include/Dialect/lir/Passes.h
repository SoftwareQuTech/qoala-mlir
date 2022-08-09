#ifndef LIR_PASSES_H_
#define LIR_PASSES_H_

namespace mlir
{

    class Pass;

    std::unique_ptr<FunctionPass> createLirReorderDownPass();
    std::unique_ptr<FunctionPass> createLirReorderUpPass();
    std::unique_ptr<FunctionPass> createLirGatePass();

#define GEN_PASS_REGISTRATION
#include "Dialect/lir/Passes.h.inc"

} // namespace mlir

#endif // LIR_PASSES_H_