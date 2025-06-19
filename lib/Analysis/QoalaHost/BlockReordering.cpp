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

        auto [blocks, qubits, result] = reordering::buildMILPFromMLIR(moduleOp);
        if (failed(result)) {
            signalPassFailure();
        }

        reordering::MILPModelBuilder model;
        if (!model.initialize()) {
            moduleOp.emitError("Failed to initialize SCIP.");
            signalPassFailure();
            return;
        }

        model.setProblemData(blocks, qubits);
        model.createVariables();
        model.addConstraints();
        model.setObjective();

        if (!model.optimize()) {
            moduleOp.emitError("MILP solve failed.");
            signalPassFailure();
            return;
        }

        for (const auto &block: blocks) {
            for (const auto *op: block->getOperations()) {
                double start = model.getOperationStartTime(op->getId());
                LLVM_DEBUG(llvm::dbgs() << "Start[" << op->getId() << "] = " << start << "\n");
            }
        }

        model.cleanup();
    }
} // namespace qoala::analysis
