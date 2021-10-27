#ifndef LIR_PASSES_H_
#define LIR_PASSES_H_

namespace mlir
{

    class Pass;

    std::unique_ptr<FunctionPass> createLirRewritePass();

#define GEN_PASS_REGISTRATION
#include "Dialect/lir/Passes.h.inc"

} // namespace mlir

#endif // LIR_PASSES_H_