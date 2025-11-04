#include "Dialect/QNet/Passes.h"
#include "Dialect/QNet/QNet.h"
#include "llvm/Support/Debug.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"

#define DEBUG_TYPE "qnet-peephole-optimizations-pass"

using namespace mlir;
using namespace qoala::dialects::qnet;

namespace qoala::analysis {
#define GEN_PASS_DEF_QNETPEEPHOLEOPTIMIZATIONS
#include "Dialect/QNet/Passes.h.inc"

    class QNetPeepholeOptimizationsPass : public impl::QNetPeepholeOptimizationsBase<QNetPeepholeOptimizationsPass> {
        using QNetPeepholeOptimizationsBase::QNetPeepholeOptimizationsBase;
        void runOnOperation() override;
    };

    void QNetPeepholeOptimizationsPass::runOnOperation() {
        LLVM_DEBUG(llvm::dbgs() << "[QNet][Peephole Optimizations] starts, hermitian cancellations="
                                << this->hermiatianCancellations << "\n");
    }
} /* namespace qoala::analysis */
