#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/QoalaHost/Passes.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Tools/QoalaOpt.h"
#include "llvm/Support/Debug.h"
#include "mlir/IR/Diagnostics.h"

#define DEBUG_TYPE "qoalahost-reorder-blocks-pass"

using namespace mlir;
using namespace qoala::dialects;
using namespace qoala::analysis;
using namespace qoala::options;

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
                                << "ns, host_instr_time=" << qoalaOptHostInstrTime << "ns, host_peer_latency="
                                << qoalaOptHostPeerLatency << "ns, qnos_instr_time=" << qoalaOptQNosInstrTime
                                << "ns, qubits_lifetime=" << qoalaOptQubitLifetime
                                << "ns, with-deadlines=" << this->withDeadlines << "\n");

        ModuleOp moduleOp = this->getOperation();

        auto [blocks, qubits, precedences, idToBlockMap, result] = reordering::buildMILPFromMLIR(moduleOp);
        if (failed(result)) {
            signalPassFailure();
        }

        reordering::MILPBlockOrderModel blockOrderModel;
        if (!blockOrderModel.initialize()) {
            moduleOp.emitError("Failed to initialize SCIP.");
            signalPassFailure();
        }

        blockOrderModel.setProblemData(blocks, qubits, precedences);
        blockOrderModel.createVariables();
        blockOrderModel.addConstraints();
        blockOrderModel.setObjective();

        if (!blockOrderModel.optimize()) {
            moduleOp.emitError("MILP solve failed.");
            signalPassFailure();
        }

        std::vector<std::string> orderedBlockIds = blockOrderModel.getOrderedBlocks();

        blockOrderModel.cleanup();

        if (this->withDeadlines) {
            reordering::BlockPrecedenceList deadlinesPrecedences =
                    createPrecedenceFromOrder(&moduleOp, orderedBlockIds, idToBlockMap);

            reordering::MILPBlockDeadlineModel deadlineModel;
            if (!deadlineModel.initialize()) {
                moduleOp.emitError("Failed to initialize SCIP.");
                signalPassFailure();
            }

            deadlineModel.setProblemData(blocks, qubits, deadlinesPrecedences);
            deadlineModel.createVariables();
            deadlineModel.addConstraints();
            deadlineModel.setObjective();

            if (!deadlineModel.optimize()) {
                moduleOp.emitError("MILP solve failed.");
                signalPassFailure();
            }

            if (!deadlineModel.checkSolverStatus(&moduleOp)) {
                signalPassFailure();
            }

            auto [deadlines, refBlockId] = deadlineModel.computeBlockDeadlines();

            deadlineModel.cleanup();

            reordering::annotateBlockDeadlines(moduleOp, deadlines, refBlockId);
        }

        LogicalResult status = reordering::reorderBlocksByMilpOrder(moduleOp, orderedBlockIds);
        if (failed(status)) {
            signalPassFailure();
        }

        // Remove all the qoalahost::NopOp which were only here to model the PostTasks.
        // We cannot leave them as a qoalahost::CallOps must always be the last operation of its block.
        auto mainFuncs = moduleOp.getOps<qoalahost::MainFuncOp>();
        if (mainFuncs.empty()) {
            emitError(moduleOp.getLoc(), "No main function found in module");
            signalPassFailure();
        }
        qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();
        mainFunc.walk([](qoalahost::NopOp nop) { nop.erase(); });
    }
} // namespace qoala::analysis
