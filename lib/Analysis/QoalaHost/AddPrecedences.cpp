#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/QoalaHost/Passes.h"
#include "llvm/Support/Debug.h"
#include "mlir/IR/BuiltinOps.h"

#define DEBUG_TYPE "qoalahost-add-precedences-pass"

using namespace mlir;
using namespace qoala::analysis;

namespace qoala::analysis {
#define GEN_PASS_DEF_QOALAHOSTADDBLOCKPRECEDENCES
#include "Dialect/QoalaHost/Passes.h.inc"

    class QoalaHostAddBlockPrecedencesPass
        : public impl::QoalaHostAddBlockPrecedencesBase<QoalaHostAddBlockPrecedencesPass> {
        using QoalaHostAddBlockPrecedencesBase::QoalaHostAddBlockPrecedencesBase;
        void runOnOperation() override;
    };

    void QoalaHostAddBlockPrecedencesPass::runOnOperation() {
        ModuleOp module = this->getOperation();
        LLVM_DEBUG(llvm::dbgs() << "QoalaHostAddBlockPrecedencesPass\n");
        if (failed(precedences::addPrecedences(module, this->useOnlineScheduler))) {
            signalPassFailure();
        }
    }
} // namespace qoala::analysis
