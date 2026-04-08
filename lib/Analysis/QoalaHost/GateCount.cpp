#include "Analysis/QoalaHost/GateCount.h"
#include "Analysis/NetQASM/Helpers.h"
#include "Analysis/QoalaHost/AnalysisTraversal.h"
#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "Dialect/QoalaHost/Passes.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/TypeSwitch.h"
#include "llvm/Support/Debug.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/IR/BuiltinOps.h"
#define DEBUG_TYPE "qoalahost-gate-count"

using namespace mlir;
using namespace qoala;

namespace qoala::analysis::qoalahost::gatecount {

    // Mutable state threaded through the CFG traversal.
    struct GateCountState {
        llvm::DenseMap<Value, std::string> qubitIds;
        llvm::StringMap<uint32_t> tempOneQubitGateCount;
        uint32_t gateCount = 0;
        uint32_t oneQubitGateCount = 0;
        uint32_t twoQubitGateCount = 0;
        llvm::StringMap<uint32_t> detailedGateCount;
        llvm::StringMap<uint32_t> detailedOneQubitGateCount;
        llvm::StringMap<uint32_t> detailedTwoQubitGateCount;
    };

    // Map call operands/results to callee block args/returns.
    static DenseMap<Value, Value> mapReturnValToCaller(dialects::qoalahost::CallOp callOp, FunctionOpInterface callee) {
        DenseMap<Value, Value> returnedValuesMap;

        // Map return values
        const auto returnOps = callee.front().getOps<dialects::netqasm::ReturnOp>();
        assert(!returnOps.empty() && "GateCount: Callee must have a ReturnOp");
        const dialects::netqasm::ReturnOp returnOp = *returnOps.begin();
        for (auto [index, retVal] : llvm::enumerate(returnOp->getOperands())) {
            returnedValuesMap.try_emplace(retVal, callOp.getResult(index));
        }

        return returnedValuesMap;
    }

    /**
     * Flush accumulated one-qubit gates for a qubit into global counters.
     * One-qubit gates are buffered and only committed when a measurement or
     * two-qubit gate is encountered, because the gate count is tracked up to
     * measurement or last two-qubit op for fidelity estimation.
     */
    static void flushTempOneQubitCount(const StringRef &qId, GateCountState &state) {

        LLVM_DEBUG(llvm::dbgs() << "Looking for qubit " << qId << " in temp one-qubit gate counts.\n");
        if (state.tempOneQubitGateCount.contains(qId)) {
            LLVM_DEBUG(llvm::dbgs() << "Found qubit " << qId << " in temp one-qubit gate counts.\n");
            const uint32_t tempCount = state.tempOneQubitGateCount.at(qId);
            if (tempCount != 0) {
                LLVM_DEBUG(llvm::dbgs() << "Flushing one-qubit gates count on qubit " << qId << ".\n");
                state.gateCount += tempCount;
                state.oneQubitGateCount += tempCount;
                state.detailedGateCount[qId] += tempCount;
                state.detailedOneQubitGateCount[qId] += tempCount;
                state.tempOneQubitGateCount[qId] = 0;
            }
        }
    }

    static std::optional<Value> getCallValFromCallee(const Value qubit, const netqasm::ArgValueMap &valuesArgMap,
                                                     const DenseMap<Value, Value> &returnedValuesMap) {
        if (const auto blockArg = dyn_cast<BlockArgument>(qubit)) {
            return valuesArgMap.getCallerValueForArg(blockArg);
        }
        if (returnedValuesMap.contains(qubit)) {
            return returnedValuesMap.at(qubit);
        }

        return std::nullopt;
    }

    static StringRef getQubitID(const Value &operand, const llvm::DenseMap<Value, std::string> &qubitIDs,
                                const netqasm::ArgValueMap &valuesArgMap,
                                const DenseMap<Value, Value> &returnedValuesMap) {
        const auto callVal = getCallValFromCallee(operand, valuesArgMap, returnedValuesMap);
        assert(callVal.has_value() && "GateCount: Untracked qubit!");
        return qubitIDs.at(callVal.value());
    }

    /**
     * Process gate operations from a callee and update gate counts.
     * One-qubit gates are buffered in tempOneQubitGateCount and only flushed
     * (committed to gateCount/oneQubitGateCount) when a measurement or two-qubit
     * gate is encountered. Two-qubit gates are counted immediately since they are
     * themselves the flush trigger. This ensures gate counts reflect only gates
     * up to the last measurement or two-qubit op (for fidelity estimation).
     */
    static void processCalleeGates(FunctionOpInterface callee, dialects::qoalahost::CallOp callOp,
                                   const std::string &blockId, GateCountState &state) {
        uint32_t opIdx = 1;
        netqasm::ArgValueMap valuesArgMap = netqasm::getRoutineArgValues(callee, callOp.getOperands());
        auto returnedValuesMap = mapReturnValToCaller(callOp, callee);

        callee.walk([&](Operation *innerOp) {
            llvm::TypeSwitch<Operation *>(innerOp)
                    .Case<dialects::netqasm::MeasureOp>([&](auto meas) {
                        auto operand = meas.getOperation()->getOperands().front();
                        auto qId = getQubitID(operand, state.qubitIds, valuesArgMap, returnedValuesMap);
                        LLVM_DEBUG(llvm::dbgs() << "Found Meas op on qubit " << qId << ".\n");
                        flushTempOneQubitCount(qId, state);
                    })
                    .Case<dialects::netqasm::QInitOp, dialects::netqasm::EprsOp>([&](auto init) {
                        auto operand = init.getOperation()->getOperands().front();
                        std::string qId = blockId + "::" + std::to_string(opIdx);
                        LLVM_DEBUG(llvm::dbgs() << "Found initialization of qubit " << qId << ".\n");
                        if (auto newQubit = getCallValFromCallee(operand, valuesArgMap, returnedValuesMap)) {
                            state.qubitIds[newQubit.value()] = qId;
                        } else {
                            LLVM_DEBUG(llvm::dbgs() << "Qubit " << qId << " is initialized but not returned.\n");
                            returnedValuesMap[operand] = operand;
                            state.qubitIds[operand] = qId;
                        }
                        state.tempOneQubitGateCount[qId] = 0;
                        state.detailedOneQubitGateCount[qId] = 0;
                        state.detailedTwoQubitGateCount[qId] = 0;
                        state.detailedGateCount[qId] = 0;
                    })
                    .Case<dialects::netqasm::ifaces::SingleQubitOp>([&](auto oneQubitOp) {
                        auto operand = oneQubitOp.getOperation()->getOperands().front();
                        auto qId = getQubitID(operand, state.qubitIds, valuesArgMap, returnedValuesMap);
                        LLVM_DEBUG(llvm::dbgs() << "Found one-qubit gate " << oneQubitOp << ".\n");
                        LLVM_DEBUG(llvm::dbgs() << "Updating one-qubit gate count on qubit " << qId << ".\n");
                        state.tempOneQubitGateCount[qId] += 1;
                    })
                    .Case<dialects::netqasm::ifaces::DualQubitOp>([&](auto twoQubitOp) {
                        state.gateCount++;
                        state.twoQubitGateCount++;
                        LLVM_DEBUG(llvm::dbgs() << "Found two-qubit gate " << twoQubitOp.getOperation() << ".\n");
                        for (auto operand : twoQubitOp.getOperation()->getOperands()) {
                            auto qId = getQubitID(operand, state.qubitIds, valuesArgMap, returnedValuesMap).str();
                            LLVM_DEBUG(llvm::dbgs() << "Updating two-qubit gate count on qubit " << qId << ".\n");
                            state.detailedGateCount[qId] += 1;
                            state.detailedTwoQubitGateCount[qId] += 1;
                            flushTempOneQubitCount(qId, state);
                        }
                    });
            ++opIdx;
        });
    }

    /**
     * Traverse CFG and count gates.
     * For conditional branches, evaluates both paths and selects the one with more gates.
     * For blocks without explicit control-flow terminators, uses scanForReadyBlock
     * to determine the next block based on blk_meta prerequisites and physical order.
     */
    static void traverseCFGAndCountGates(mlir::Block *startBlock, llvm::DenseSet<mlir::Block *> &visited,
                                         GateCountState &state, const BlockPrerequisites &prereqs,
                                         llvm::DenseSet<mlir::Block *> condBrTargets) {
        Block *block = startBlock;
        while (block) {
            if (visited.contains(block)) {
                return;
            }
            visited.insert(block);

            // Extract block ID
            auto blkMeta = dyn_cast<dialects::qoalahost::BlkMeta>(block->front());
            assert(blkMeta && "GateCount: could not find BlkMeta operation on the block.");
            std::string blockId = blkMeta.getBlockId().str();

            LLVM_DEBUG(llvm::dbgs() << "Visiting block: " << blockId << ".\n");

            // Process all operations in current block except terminator
            for (auto &op : block->getOperations()) {
                if (isa<cf::CondBranchOp, cf::BranchOp>(op)) {
                    break;
                }

                if (auto callOp = dyn_cast<dialects::qoalahost::CallOp>(&op)) {
                    auto callee = callOp.getCalleeOperation<FunctionOpInterface>();
                    processCalleeGates(callee, callOp, blockId, state);
                }
            }

            // Handle branching via terminator
            auto terminator = block->getTerminator();
            if (auto condBr = dyn_cast<cf::CondBranchOp>(terminator)) {
                LLVM_DEBUG(llvm::dbgs() << "Evaluating branch from " << blockId << ".\n");
                // Capture initial state
                GateCountState initialState = state;

                // Forbid both branch destinations from scanForReadyBlock
                llvm::DenseSet<Block *> innerForbidden = condBrTargets;
                innerForbidden.insert(condBr.getTrueDest());
                innerForbidden.insert(condBr.getFalseDest());

                // Analyze true branch
                llvm::DenseSet<Block *> visitedTrue = visited;
                traverseCFGAndCountGates(condBr.getTrueDest(), visitedTrue, state, prereqs, innerForbidden);
                GateCountState trueState = state;
                LLVM_DEBUG(llvm::dbgs() << "TRUE branch from " << blockId << " evaluated: " << trueState.gateCount
                                        << ".\n");

                // Restore initial state and analyze false branch
                state = initialState;
                llvm::DenseSet<Block *> visitedFalse = visited;
                traverseCFGAndCountGates(condBr.getFalseDest(), visitedFalse, state, prereqs, innerForbidden);
                LLVM_DEBUG(llvm::dbgs() << "FALSE branch from " << blockId << " evaluated: " << state.gateCount
                                        << ".\n");

                // Branch comparison.
                // Use only committed (already flushed) gates, since temp one-qubit gates
                // that never reach a measurement/two-qubit interaction should not contribute.
                if (trueState.gateCount >= state.gateCount) {
                    state = trueState;
                    visited = visitedTrue;
                } else {
                    // state already contains false branch results, just update visited set
                    visited = visitedFalse;
                }
                return;
            }

            if (auto br = dyn_cast<cf::BranchOp>(terminator)) {
                block = br.getDest();
                continue;
            }

            // No explicit cf terminator: find next ready block via blk_meta prerequisites.
            block = scanForReadyBlock(*startBlock->getParent(), visited, prereqs, condBrTargets);
        }
    }

    /**
     * Count the gates qubit-wise, and grouped by one-qubit and two-qubit gates
     * as they have different error rates and so contribute differently
     * to the fidelity of each qubit
     */
    QoalaHostGateCount::QoalaHostGateCount(Operation *op) {

        // Current implementation is tightly coupled with fidelity estimation, i.e.,
        // gate count is tracked up to measurement or last two-qubit op.
        // In future could change to track every qubit up to last quantum op.

        LLVM_DEBUG(llvm::dbgs() << "Running QoalaHostGateCountPass\n");

        auto moduleOp = dyn_cast<ModuleOp>(op);
        const auto mainFuncs = moduleOp.getOps<dialects::qoalahost::MainFuncOp>();
        assert(!mainFuncs.empty() && "GateCount: No main function found in module.");

        dialects::qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();

        // Build prerequisite maps directly from blk_meta attributes.
        // Avoid buildMILPFromMLIR because it mutates the IR (inserts synthetic nop ops),
        // which would break getTerminator() used later in the traversal.
        BlockPrerequisites prereqs = buildBlockPrerequisites(mainFunc);

        // Traverse starting from the entry block. scanForReadyBlock handles finding
        // all reachable blocks based on blk_meta prerequisites and physical order.
        GateCountState state;
        llvm::DenseSet<Block *> visited;
        llvm::DenseSet<Block *> condBrTargets;
        traverseCFGAndCountGates(&mainFunc.front(), visited, state, prereqs, condBrTargets);

        // Copy results to class members
        gateCount = state.gateCount;
        oneQubitGateCount = state.oneQubitGateCount;
        twoQubitGateCount = state.twoQubitGateCount;
        detailedGateCount = std::move(state.detailedGateCount);
        detailedOneQubitGateCount = std::move(state.detailedOneQubitGateCount);
        detailedTwoQubitGateCount = std::move(state.detailedTwoQubitGateCount);
    }

} // namespace qoala::analysis::qoalahost::gatecount
