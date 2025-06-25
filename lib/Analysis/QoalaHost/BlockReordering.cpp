#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/QoalaHost/Passes.h"
#include "mlir/IR/Diagnostics.h"
#include "llvm/Support/Debug.h"
#include "Tools/QoalaOpt.h"
#include "Dialect/QoalaHost/QoalaHost.h"

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
                                << "ns, two_gate_duration=" << qoalaOptTwoGateDuration
                                << "ns, latency=" << qoalaOptLatency << "ns, link_duration=" << qoalaOptLinkDuration
                                << "ns, host_instr_time=" << qoalaOptHostInstrTime
                                << "ns, host_peer_latency=" << qoalaOptHostPeerLatency
                                << "ns, qnos_instr_time=" << qoalaOptQNosInstrTime << "ns\n");

        ModuleOp moduleOp = this->getOperation();

        auto [blocks, qubits, precedences, result] = reordering::buildMILPFromMLIR(moduleOp);
        if (failed(result)) {
            signalPassFailure();
        }

        // reordering::MILPModelBuilder model;
        // if (!model.initialize()) {
        //     moduleOp.emitError("Failed to initialize SCIP.");
        //     signalPassFailure();
        //     return;
        // }

        // model.setProblemData(blocks, qubits);
        // model.createVariables();
        // model.addConstraints();
        // model.setObjective();

        // if (!model.optimize()) {
        //     moduleOp.emitError("MILP solve failed.");
        //     signalPassFailure();
        //     return;
        // }

        // for (const auto &block: blocks) {
        //     for (const auto *op: block->getOperations()) {
        //         double start = model.getOperationStartTime(op->getId());
        //         LLVM_DEBUG(llvm::dbgs() << "Start[" << op->getId() << "] = " << start << "\n");
        //     }
        // }

        // model.cleanup();

        // Remove all the qoalahost::NopOp which were only here to model the PostTasks.
        // We cannot leave them as a qoalahost::CallOps must always be the last operation of its block.
        auto mainFuncs = moduleOp.getOps<qoalahost::MainFuncOp>();
        if (mainFuncs.empty()) {
            mlir::emitError(moduleOp.getLoc(), "No main function found in module");
            signalPassFailure();
        }
        qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();
        mainFunc.walk([](qoalahost::NopOp nop) { nop.erase(); });
    }
} // namespace qoala::analysis
