#ifndef HIR_PASSES_H_
#define HIR_PASSES_H_

namespace mlir
{

    class Pass;

    std::unique_ptr<FunctionPass> createHirRewritePass();

#define GEN_PASS_REGISTRATION
#include "Dialect/hir/Passes.h.inc"

} // namespace mlir

#endif // HIR_PASSES_H_