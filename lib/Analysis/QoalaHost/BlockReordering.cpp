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

        reordering::MILPBlockOrderModel blockOrderPrimary;
        if (!blockOrderPrimary.initialize()) {
            moduleOp.emitError("Failed to initialize SCIP (primary).");
            signalPassFailure();
        }

        blockOrderPrimary.setProblemData(blocks, qubits, precedences);
        blockOrderPrimary.createVariables();
        blockOrderPrimary.addConstraints();
        blockOrderPrimary.setPrimaryObjective();

        if (!blockOrderPrimary.optimize() || !blockOrderPrimary.checkSolverStatus(&moduleOp)) {
            moduleOp.emitError("MILP solve (primary) failed.");
            signalPassFailure();
        }

        // read optimal primary scalar as double
        double zStar = blockOrderPrimary.getPrimaryObjectiveValueFromSolution();

        // we also need the *schedule state* (start times) later to reorder blocks if
        // the second pass ends up with the same start times but different tie-breaks.
        // BUT actually we only need final block order from *secondary*, not from primary,
        // so we don't extract ordering yet.
        blockOrderPrimary.cleanup();

        reordering::MILPBlockOrderModel blockOrderSecondary;
        if (!blockOrderSecondary.initialize()) {
            moduleOp.emitError("Failed to initialize SCIP (secondary).");
            signalPassFailure();
        }

        blockOrderSecondary.setProblemData(blocks, qubits, precedences);
        blockOrderSecondary.createVariables();
        blockOrderSecondary.addConstraints();

        // now: lock the primary optimum
        blockOrderSecondary.constrainPrimaryObjectiveTo(zStar);

        // now: set the deterministic secondary objective
        blockOrderSecondary.setSecondaryObjectiveDeterministic();

        if (!blockOrderSecondary.optimize() || !blockOrderSecondary.checkSolverStatus(&moduleOp)) {
            moduleOp.emitError("MILP solve (secondary) failed.");
            signalPassFailure();
        }

        // final canonical block order:
        std::vector<std::string> orderedBlockIds = blockOrderSecondary.getOrderedBlocks();

        blockOrderSecondary.cleanup();

        // if (this->withDeadlines) {
        //     reordering::BlockPrecedenceList deadlinesPrecedences =
        //             createPrecedenceFromOrder(&moduleOp, orderedBlockIds, idToBlockMap);

        //     reordering::MILPBlockDeadlineModel deadlineModel;
        //     if (!deadlineModel.initialize()) {
        //         moduleOp.emitError("Failed to initialize SCIP.");
        //         signalPassFailure();
        //     }

        //     deadlineModel.setProblemData(blocks, qubits, deadlinesPrecedences);
        //     deadlineModel.createVariables();
        //     deadlineModel.addConstraints();
        //     deadlineModel.setObjective();

        //     if (!deadlineModel.optimize()) {
        //         moduleOp.emitError("MILP solve failed.");
        //         signalPassFailure();
        //     }

        //     if (!deadlineModel.checkSolverStatus(&moduleOp)) {
        //         signalPassFailure();
        //     }

        //     auto [deadlines, refBlockId] = deadlineModel.computeBlockDeadlines();

        //     deadlineModel.cleanup();

        //     reordering::annotateBlockDeadlines(moduleOp, deadlines, refBlockId);
        // }

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
