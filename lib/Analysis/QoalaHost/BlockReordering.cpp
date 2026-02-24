#include "Analysis/QoalaHost/GateCount.h"
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

        // If we skip secondary, we still need an order:
        std::vector<std::string> orderedBlockIds;

        if (qoalaOptUnoptimize) {
            // In unoptimize mode, do NOT apply the deterministic secondary tie-break.
            // Use the primary schedule directly (it already reflects the flipped objective).
            orderedBlockIds = blockOrderPrimary.getOrderedBlocks();
            blockOrderPrimary.cleanup();
        } else {
            // Optimize mode: keep deterministic tie-break that enforces
            // "earlier alloc => earlier meas" among primary optima.
            auto allocOrder = blockOrderPrimary.computeAllocationOrderFromSolution();
            blockOrderPrimary.cleanup();

            reordering::MILPBlockOrderModel blockOrderSecondary;
            if (!blockOrderSecondary.initialize()) {
                moduleOp.emitError("Failed to initialize SCIP (secondary).");
                signalPassFailure();
            }

            blockOrderSecondary.setProblemData(blocks, qubits, precedences);
            blockOrderSecondary.createVariables();
            blockOrderSecondary.addConstraints();

            // lock the primary optimum
            blockOrderSecondary.constrainPrimaryObjectiveTo(zStar);

            // Inject allocation order from primary solution
            blockOrderSecondary.setPrimaryAllocationOrder(allocOrder);

            // deterministic secondary objective (meas early for earlier allocs)
            blockOrderSecondary.setSecondaryObjectiveDeterministic();

            if (!blockOrderSecondary.optimize() || !blockOrderSecondary.checkSolverStatus(&moduleOp)) {
                moduleOp.emitError("MILP solve (secondary) failed.");
                signalPassFailure();
            }

            orderedBlockIds = blockOrderSecondary.getOrderedBlocks();
            blockOrderSecondary.cleanup();
        }

        LogicalResult status = reordering::reorderBlocksByMilpOrder(moduleOp, orderedBlockIds);
        if (failed(status)) {
            signalPassFailure();
        }

        if (this->withDeadlines) {
            reordering::BlockPrecedenceList deadlinesPrecedences =
                    createPrecedenceFromOrder(&moduleOp, orderedBlockIds, idToBlockMap);

            // Phase 1: maximize gmin
            reordering::MILPBlockDeadlineModel deadlinePrimary;
            if (!deadlinePrimary.initialize()) {
                moduleOp.emitError("Failed to initialize SCIP (deadline primary).");
                signalPassFailure();
            }
            deadlinePrimary.setProblemData(blocks, qubits, deadlinesPrecedences);
            deadlinePrimary.createVariables();
            deadlinePrimary.addConstraints();
            deadlinePrimary.setPrimaryObjective();
            if (!deadlinePrimary.optimize() || !deadlinePrimary.checkSolverStatus(&moduleOp)) {
                moduleOp.emitError("Deadlines MILP solve (primary) failed.");
                signalPassFailure();
            }
            const double gminStar = deadlinePrimary.getPrimaryObjectiveValueFromSolution();
            deadlinePrimary.cleanup();

            // Phase 2: fix gmin == gmin* and apply deterministic tie-breaker
            reordering::MILPBlockDeadlineModel deadlineSecondary;
            if (!deadlineSecondary.initialize()) {
                moduleOp.emitError("Failed to initialize SCIP (deadline secondary).");
                signalPassFailure();
            }
            deadlineSecondary.setProblemData(blocks, qubits, deadlinesPrecedences);
            deadlineSecondary.createVariables();
            deadlineSecondary.addConstraints();
            deadlineSecondary.constrainPrimaryObjectiveTo(gminStar);
            deadlineSecondary.setSecondaryObjectiveDeterministic();
            if (!deadlineSecondary.optimize() || !deadlineSecondary.checkSolverStatus(&moduleOp)) {
                moduleOp.emitError("Deadlines MILP solve (secondary) failed.");
                signalPassFailure();
            }

            auto [deadlines, refBlockId] = deadlineSecondary.computeBlockDeadlines();
            deadlineSecondary.cleanup();

            reordering::annotateBlockDeadlines(moduleOp, deadlines, refBlockId);
        }

        // Remove all the qoalahost::NopOp which were only here to model the PostTasks.
        // We cannot leave them as a qoalahost::CallOps must always be the last operation of its block.
        auto mainFuncs = moduleOp.getOps<dialects::qoalahost::MainFuncOp>();
        if (mainFuncs.empty()) {
            emitError(moduleOp.getLoc(), "No main function found in module");
            signalPassFailure();
        }
        dialects::qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();
        mainFunc.walk([](dialects::qoalahost::NopOp nop) { nop.erase(); });

        // Preserve gate count analysis
        markAnalysesPreserved<qoalahost::gatecount::QoalaHostGateCount>();
    }
} // namespace qoala::analysis
