#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/QoalaHost/Passes.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "qoalahost-add-deps-pass"

using namespace mlir;
using namespace qoala::analysis;

namespace qoala::analysis {
#define GEN_PASS_DEF_QOALAHOSTADDBLOCKDEPENDENCIES
#include "Dialect/QoalaHost/Passes.h.inc"

    class QoalaHostAddBlockDependenciesPass
        : public impl::QoalaHostAddBlockDependenciesBase<QoalaHostAddBlockDependenciesPass> {
        using QoalaHostAddBlockDependenciesBase::QoalaHostAddBlockDependenciesBase;
        void runOnOperation() override;
    };

    void QoalaHostAddBlockDependenciesPass::runOnOperation() {
        ModuleOp module = dyn_cast<ModuleOp>(getOperation());
        assert(module); // We expect the cast to succeed
        LLVM_DEBUG(llvm::dbgs() << "QoalaHostAddBlockDependenciesPass\n");
        dependencies::addDependencies(module);
    }
} // namespace qoala::analysis
