#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/QoalaHost/Passes.h"
#include "mlir/IR/Diagnostics.h"
#include "llvm/Support/Debug.h"
#include "Tools/QoalaOpt.h"

#define DEBUG_TYPE "qoalahost-reorder-blocks-pass"

using namespace mlir;
using namespace qoala::dialects;
using namespace qoala::analysis;

namespace qoala::analysis {
#define GEN_PASS_DEF_QOALAHOSTREORDERBLOCKS
#include "Dialect/QoalaHost/Passes.h.inc"

    class QoalaHostReorderBlocksPass : public impl::QoalaHostReorderBlocksBase<QoalaHostReorderBlocksPass> {
        using QoalaHostReorderBlocksBase::QoalaHostReorderBlocksBase;
        void runOnOperation() override;
    };

    void QoalaHostReorderBlocksPass::runOnOperation() {
        LLVM_DEBUG(llvm::dbgs() << "QoalaHostReorderBlocksPass: single_gate_duration=" << qoalaOptSingleGateDuration
                                << "ns, two_gate_duration=" << qoalaOptTwoGateDuration << "ns, latency="
                                << qoalaOptLatency << "ns, link_duration=" << qoalaOptLinkDuration << "ns\n");

        ModuleOp moduleOp = this->getOperation();
        std::pair<mlir::LogicalResult, std::vector<std::unique_ptr<reordering::MILPBlock>>> result =
                reordering::buildMILPBlocks(moduleOp);
        mlir::LogicalResult status = result.first;
        std::vector<std::unique_ptr<reordering::MILPBlock>> &milpBlocks = result.second;

        if (failed(status)) {
            signalPassFailure();
            return;
        }
    }
} // namespace qoala::analysis
