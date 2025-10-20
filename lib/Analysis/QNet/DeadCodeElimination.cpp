#include "Dialect/QNet/Passes.h"
#include "Dialect/QNet/QNet.h"
#include "llvm/Support/Debug.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"

#define DEBUG_TYPE "qnet-dead-code-elimination-pass"

using namespace mlir;
using namespace qoala::dialects::qnet;

namespace qoala::analysis {
#define GEN_PASS_DEF_QNETDEADCODEELIMINATION
#include "Dialect/QNet/Passes.h.inc"

    class QNetDeadCodeEliminationPass : public impl::QNetDeadCodeEliminationBase<QNetDeadCodeEliminationPass> {
        using QNetDeadCodeEliminationBase::QNetDeadCodeEliminationBase;
        void runOnOperation() override;
    };

    void QNetDeadCodeEliminationPass::runOnOperation() { LLVM_DEBUG(llvm::dbgs() << "QNet DCE\n"); }
} /* namespace qoala::analysis */
