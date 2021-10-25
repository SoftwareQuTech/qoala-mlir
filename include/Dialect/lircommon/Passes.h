#ifndef LIRCOMMON_PASSES_H_
#define LIRCOMMON_PASSES_H_

namespace mlir
{

    class Pass;

    std::unique_ptr<FunctionPass> createLirCommonRewritePass();

#define GEN_PASS_REGISTRATION
#include "Dialect/lircommon/Passes.h.inc"

} // namespace mlir

#endif // LIRCOMMON_PASSES_H_