#include "Dialect/QoalaHost/Passes.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "qoalahost-add-deps-pass"

using namespace mlir;
using namespace qoala::dialects::qoalahost;

namespace qoala::analysis {
#define GEN_PASS_DEF_QOALAHOSTADDBLOCKDEPENDENCIES
#include "Dialect/QoalaHost/Passes.h.inc"

    class QoalaHostAddBlockDependenciesPass : public impl::QoalaHostAddBlockDependenciesBase<QoalaHostAddBlockDependenciesPass> {
        using QoalaHostAddBlockDependenciesBase::QoalaHostAddBlockDependenciesBase;
        void runOnOperation() override;
    };


    void QoalaHostAddBlockDependenciesPass::runOnOperation() {
        LLVM_DEBUG(llvm::dbgs() << "Hello world!\n");
    }
} /* namespace qoala::analysis */
