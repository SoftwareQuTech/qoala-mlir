#include <unordered_set>
#include <vector>

#include "Analysis/Helpers/Helpers.h"
#include "Analysis/NetQASM/Helpers.h"
#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "llvm/Support/Debug.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlow.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/IR/BuiltinOps.h"

#define DEBUG_TYPE "qoalahost-add-precedences-pass-internal"

#if __cplusplus >= 202002L
std::string blockIDFmt = "block_{}";
#else
std::string blockIDFmt = "block_%d";
#endif

using namespace mlir;
using namespace qoala::dialects;
using namespace qoala::analysis;

namespace qoala::analysis::precedences {
    // caller block → all memory effects from its callee
    struct MemEffectInfo {
        MemEffectInfo(Operation *op, std::vector<MemoryEffects::EffectInstance> effects):
            calleeOp(op), effects(std::move(effects)) { }

        MemEffectInfo(): calleeOp(nullptr), effects({}) { }

        Operation *calleeOp;
        std::vector<MemoryEffects::EffectInstance> effects;
    };

    static std::vector<MemoryEffects::EffectInstance> computeMemoryEffects(FunctionOpInterface &calleeRoutineOp,
                                                                           qoalahost::CallOp &callOp) {
        std::vector<MemoryEffects::EffectInstance> allEffects;
        const netqasm::ArgValueMap valuesArgMap =
                netqasm::getRoutineArgValues(calleeRoutineOp.getOperation(), callOp.getOperands());

        LLVM_DEBUG(llvm::dbgs() << "[MemEffects] Inspecting callee: " << callOp->getName() << "\n");

        for (auto &calleeBlock : calleeRoutineOp) {
            LLVM_DEBUG(llvm::dbgs() << "  [Block] In callee block @" << &calleeBlock << "\n");

            for (Operation &innerOp : calleeBlock.getOperations()) {
                LLVM_DEBUG(llvm::dbgs() << "    [Op] " << innerOp.getName() << "\n");

                if (auto effectOp = dyn_cast<MemoryEffectOpInterface>(&innerOp)) {
                    LLVM_DEBUG(llvm::dbgs() << "      -> Implements MemoryEffectOpInterface\n");

                    SmallVector<MemoryEffects::EffectInstance, 4> effects;
                    effectOp.getEffects(effects);

                    for (const MemoryEffects::EffectInstance &eff : effects) {
                        Value origVal = eff.getValue();
                        Value &resolvedVal = origVal;

                        // If it's a block argument, resolve to actual caller operand
                        if (auto blockArg = dyn_cast<BlockArgument>(origVal); blockArg && origVal) {
                            resolvedVal = valuesArgMap.getCallerValueForArg(blockArg);
                        }

                        const MemoryEffects::Effect *kind = eff.getEffect();
                        LLVM_DEBUG(llvm::dbgs() << "        -> Effect: "
                                                << (isa<MemoryEffects::Read>(kind)       ? "Read"
                                                    : isa<MemoryEffects::Write>(kind)    ? "Write"
                                                    : isa<MemoryEffects::Allocate>(kind) ? "Allocate"
                                                    : isa<MemoryEffects::Free>(kind)     ? "Free"
                                                                                         : "Unknown")
                                                << (resolvedVal ? " on value " : " on <null>")
                                                << (resolvedVal ? resolvedVal : Value{}) << "\n");

                        if (resolvedVal) {
                            allEffects.emplace_back(eff.getEffect(), resolvedVal, eff.getResource());
                        }
                    }
                } else {
                    LLVM_DEBUG(llvm::dbgs() << "      -> Does NOT implement MemoryEffectOpInterface\n");
                }
            }
        }

        return allEffects;
    }

    static void resolveMemorySideEffects(llvm::DenseMap<Block *, MemEffectInfo> &callSiteEffects,
                                         llvm::DenseMap<Block *, std::string> &blockIdMap,
                                         llvm::DenseMap<Block *, std::unordered_set<Block *>> &blockDeps) {
        // === Resolve memory side effect conflicts between call sites ===
        //
        // For each pair of distinct call sites (blockA and blockB), we check whether their
        // respective callees perform memory effects (e.g., reads/writes) on the same buffer.
        //
        // If both operate on the same value, and at least one of them performs a write,
        // this constitutes a memory conflict, and we must establish a precedence constraint.
        //
        // To avoid over-constraining the graph with unnecessary cycles, we only add a dependency
        // if blockA's call appears lexically before blockB's call (i.e., as per the IR's order),
        // thereby enforcing a consistent and deterministic ordering.
        //
        // When a conflict is detected, we record that `blockB` depends on `blockA` in `blockDeps`,
        // which will later be reflected in the `qoalahost.blk_meta` ops.
        for (const auto &[blockA, infoA] : callSiteEffects) {
            for (const auto &[blockB, infoB] : callSiteEffects) {
                if (blockA == blockB) {
                    continue;
                }

                LLVM_DEBUG(llvm::dbgs() << "[EffectConflict] Comparing effects between " << blockIdMap[blockA]
                                        << " and " << blockIdMap[blockB] << "\n");

                for (const MemoryEffects::EffectInstance &effA : infoA.effects) {
                    for (const MemoryEffects::EffectInstance &effB : infoB.effects) {
                        if (!effA.getValue() || !effB.getValue()) {
                            LLVM_DEBUG(llvm::dbgs() << "  -> Skipping null value effect\n");
                            continue;
                        }

                        if (effA.getValue() != effB.getValue()) {
                            LLVM_DEBUG(llvm::dbgs() << "  -> Skipping effects on different buffers: " << effA.getValue()
                                                    << " vs " << effB.getValue() << "\n");
                            continue;
                        }

                        MemoryEffects::Effect *aKind = effA.getEffect();
                        MemoryEffects::Effect *bKind = effB.getEffect();

                        const bool aIsWrite = isa<MemoryEffects::Write>(aKind);
                        const bool bIsWrite = isa<MemoryEffects::Write>(bKind);

                        LLVM_DEBUG(llvm::dbgs() << "  -> Shared buffer " << effA.getValue()
                                                << " | A: " << (aIsWrite ? "Write" : "Read")
                                                << " | B: " << (bIsWrite ? "Write" : "Read") << "\n");

                        const bool conflict = aIsWrite || bIsWrite;

                        // Enforce lexical ordering: blockA must come before blockB
                        Operation *blockAOp = infoA.calleeOp;
                        Operation *blockBOp = infoB.calleeOp;
                        const bool isLexicalOrder = blockAOp->isBeforeInBlock(blockBOp);

                        if (conflict && isLexicalOrder && blockDeps[blockB].insert(blockA).second) {
                            LLVM_DEBUG(llvm::dbgs()
                                       << "    ==> Dependency added: " << blockIdMap[blockB] << " (caller of "
                                       << blockBOp->getName() << ") depends on " << blockIdMap[blockA] << " (caller of "
                                       << blockAOp->getName() << ") due to conflict on buffer " << effA.getValue()
                                       << " (lexical order enforced)\n");
                        }
                    }
                }
            }
        }
    }

    static void transitiveReduction(llvm::DenseMap<Block *, std::string> &blockIdMap,
                                    llvm::DenseMap<Block *, std::unordered_set<Block *>> &blockDeps) {
        // === Transitive Reduction ===
        for (auto &[block, deps] : blockDeps) {
            std::unordered_set<Block *> toRemove;

            for (Block *direct : deps) {
                if (!blockDeps.contains(direct)) {
                    continue;
                }

                for (Block *transitive : blockDeps.at(direct)) {
                    if (deps.count(transitive)) {
                        LLVM_DEBUG(llvm::dbgs() << "[TransitiveReduction] Removing indirect dep: " << blockIdMap[block]
                                                << " → " << blockIdMap[transitive] << " (already reachable via "
                                                << blockIdMap[direct] << ")\n");
                        toRemove.insert(transitive);
                    }
                }
            }

            for (Block *remove : toRemove) {
                deps.erase(remove);
            }
        }
    }

    static bool isPureCfBridgeBlock(Block &b) {
        // Ignore blk_meta ops.
        Operation *term = b.getTerminator();
        if (!term) {
            return false;
        }

        // Require terminator to be a cf branch.
        if (!isa<cf::BranchOp, cf::CondBranchOp>(term)) {
            return false;
        }

        // Check that everything else is only blk_meta (and maybe constants if you want).
        for (Operation &op : b.getOperations()) {
            if (&op == term) {
                continue;
            }
            if (isa<qoalahost::BlkMeta>(&op)) {
                continue;
            }
            // Anything else means it's not "glue".
            return false;
        }
        return true;
    }

    LogicalResult addPrecedences(ModuleOp &moduleOp, bool useOnlineScheduler) {
        // This pass builds a block-level precedence graph within the `qoalahost.main_func` by
        // tracking dependencies that determine the required execution ordering between blocks.
        //
        // For each block, we determine which other blocks must be executed beforehand based on:
        // - Control flow predecessors (i.e., explicit branches or block successors)
        // - Data dependencies (i.e., SSA value producer-consumer relationships)
        // - Classical communication operations (to enforce correct send/receive sequencing)
        // - Request routine calls (preserving ordering of semantically significant subroutines)
        // - Memory side effects in called routines (e.g., conflicting reads/writes on shared buffers)
        //
        // These dependencies are encoded in a `qoalahost.blk_meta` operation inserted at the
        // beginning of each block. This metadata can be used in downstream passes to enforce
        // correct execution order or enable scheduling optimizations.
        //
        // Notes and assumptions:
        // - Dependencies caused by memory effects are pruned of transitive edges for clarity.
        // - If a `return` operation returns a value, its use will result in SSA-based data dependencies.
        // - If a `return` returns nothing (e.g., used for structural correctness), it introduces no ordering.
        //   It is the responsibility of later passes that modify block ordering to preserve this property.
        // - This pass assumes that control/data flow structure is finalized and dead/unreachable code has
        //   been removed. It also assumes return ops are at block ends and are not followed by other ops.

        LLVM_DEBUG(llvm::dbgs() << "\n=== QoalaHostPrecedences: "
                                   "Building Block Dependency Graph ===\n");

        // Block-level dependency graph: block -> set of blocks it depends on.
        // Note: we use an `std::unordered_set` rather than `llvm::SmallPtrSet` because we cannot estimate the size of
        // this set. Depending on the program it could grow large.
        llvm::DenseMap<Block *, std::unordered_set<Block *>> blockDeps;

        // Block -> string ID (e.g., block_0)
        llvm::DenseMap<Block *, std::string> blockIdMap;
        uint32_t idCounter = 0;

        const auto mainFuncs = moduleOp.getOps<qoalahost::MainFuncOp>();
        assert(!mainFuncs.empty() && "No main func? This is embarrassing...");
        // We get the first main func op; since it's unique in the module, it's safe to "ignore" the rest of the
        // iterator
        qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();

        for (Block &block : mainFunc) {
            blockIdMap.try_emplace(&block, helpers::formatString(blockIDFmt, idCounter++));
        }

        LLVM_DEBUG(llvm::dbgs() << "\n=== Tracking all precedences ===\n");

        std::vector<Operation *> commOps;
        std::vector<Operation *> requestCallOps;

        llvm::DenseMap<Block *, MemEffectInfo> callSiteEffects;

        auto classicalCommOps = mainFunc.getOps<qoalahost::ifaces::ClassicalCommInterface>();
        for (auto classicalCommOp : classicalCommOps) {
            commOps.push_back(classicalCommOp.getOperation());
        }

        mainFunc.walk([&](qoalahost::CallOp callOp) {
            Block *consumerBlock = callOp->getBlock();
            Operation *callee = callOp.getCalleeOperation();
            // Track request routine call ops and collect memory side effects from callees.
            //
            // For each `qoalahost.call`, we resolve the callee symbol to determine if it refers to
            // a `netqasm.request_routine`. If so, we add it to the requestCallOps list.
            //
            // Regardless of whether it's a request routine, we always collect memory side effects
            // from the callee body (if it has one). This enables detection of inter-block dependencies
            // due to reads/writes on shared memory buffers.
            //
            // Notes:
            // - We resolve the symbol directly via `SymbolTable::lookupNearestSymbolFrom`. An alternative
            //   would be to precompute and index all known `netqasm.request_routine` ops, but given the
            //   expected number is small, direct resolution is more efficient and simple for now.
            // - Memory effects are aggregated per call site and later used to compute conflicts
            //   (e.g., Write–Write or Read–Write) that imply a block dependency.

            assert(callee && "Module contains a call operation to a function without definition/declaration");
            auto calleeRoutineOp = dyn_cast<FunctionOpInterface>(callee);
            if (!calleeRoutineOp) {
                callee->emitError("Called routine is not a NetQASM routine");
                return WalkResult::interrupt();
            }

            if (isa<dialects::netqasm::RequestRoutineOp>(callee)) {
                requestCallOps.push_back(callOp.getOperation());
            }

            std::vector<MemoryEffects::EffectInstance> allEffects = computeMemoryEffects(calleeRoutineOp, callOp);

            callSiteEffects[consumerBlock] = {callee, allEffects};
            return WalkResult::advance();
        });

        mainFunc.walk([&](Operation *op) {
            Block *consumerBlock = op->getBlock();
            // Track data dependencies
            for (Value operand : op->getOperands()) {
                if (Operation *producer = operand.getDefiningOp()) {
                    Block *producerBlock = producer->getBlock();
                    if (producerBlock != consumerBlock && blockDeps[consumerBlock].insert(producerBlock).second) {
                        LLVM_DEBUG(llvm::dbgs()
                                   << blockIdMap[consumerBlock] << " depends on " << blockIdMap[producerBlock] << "\n");
                    }
                }
            }
            // Add extra data dependency on the first block if the *classical comm operation* uses a remote:
            // Do we need to add data dependencies on operations dialects::netqasm::ifaces::EntangledQubitOp?
            //  (EprsOp and EprsMeasureOp). Maybe not, since they are
            if (isa<qoalahost::ifaces::ClassicalCommInterface>(op)) {
                Block *producerBlock = &mainFunc.front();
                assert(!producerBlock->getOps<qoalahost::RemoteIDRefOp>().empty());
                if (blockDeps[consumerBlock].insert(producerBlock).second) {
                    LLVM_DEBUG(llvm::dbgs()
                               << blockIdMap[consumerBlock] << " depends on " << blockIdMap[producerBlock] << "\n");
                }
            }
        });

        resolveMemorySideEffects(callSiteEffects, blockIdMap, blockDeps);

        // === Propagate dependencies to predecessors at merge points ===
        //
        // For any merge block M that has both data dependencies and control-flow
        // predecessors, we must ensure that all dependency blocks are placed
        // *before* all predecessor blocks in the final block order.
        //
        // The online scheduler walks blocks sequentially by list index, following
        // jumps for branches. If a predecessor P can jump past a dependency D
        // (because D is placed after P in the list), then D becomes unreachable
        // and any values it defines will be missing at M.
        //
        // Fix: for each dependency Di of M and each predecessor Pj of M,
        // add Di as a dependency of Pj — but only if Di appears before Pj in
        // the current block list (to avoid forward-reference cycles where a
        // predecessor that comes *before* a dependency would gain an impossible
        // constraint).
        // Build a block -> position index for the current block list order.
        llvm::DenseMap<Block *, uint32_t> blockPos;
        uint32_t pos = 0;
        for (Block &b : mainFunc) {
            blockPos[&b] = pos++;
        }

        for (Block &block : mainFunc) {
            const auto &deps = blockDeps[&block];
            if (deps.empty()) {
                continue;
            }

            for (Block *pred : block.getPredecessors()) {
                for (Block *dep : deps) {
                    if (dep != pred && blockPos[dep] < blockPos[pred] && blockDeps[pred].insert(dep).second) {
                        LLVM_DEBUG(llvm::dbgs()
                                   << "[MergePointProp] " << blockIdMap[pred] << " now depends on " << blockIdMap[dep]
                                   << " (propagated from merge block " << blockIdMap[&block] << ")\n");
                    }
                }
            }
        }

        // === Floater-to-branch propagation ===
        //
        // The online scheduler walks blocks sequentially, following jumps at
        // branches.  A "floater" block (no CFG predecessors, not a bridge block)
        // is only reachable via sequential fall-through.  If a branch block C
        // jumps past a floater F that is transitively needed by blocks after C,
        // then F becomes unreachable and values it defines will be missing.
        //
        // Part 1: If F is already before C, add F as a dependency of C so the
        //         MILP cannot reorder F after C.
        // Part 2: If F is after C but all of F's own constraints (data deps,
        //         prev_comm, prev_ent) are satisfiable before C, move F's
        //         declaration before C and add F as a dependency of C.
        //
        // Rules:
        //  - NEVER move blocks with CFG predecessors (pred_begin != pred_end).
        //  - NEVER move bridge blocks (isPureCfBridgeBlock) — they get
        //    synthesised predecessors from getPrevNode().
        //  - ONLY add to dependencies; never modify predecessors.

        // Build prev_comm and prev_ent maps for constraint checking in Part 2.
        llvm::DenseMap<Block *, Block *> prevCommMap;
        for (size_t i = 1; i < commOps.size(); i++) {
            Block *cur = commOps[i]->getBlock();
            Block *prev = commOps[i - 1]->getBlock();
            if (prev != cur) {
                prevCommMap.try_emplace(cur, prev);
            }
        }
        llvm::DenseMap<Block *, Block *> prevEntMap;
        for (size_t i = 1; i < requestCallOps.size(); i++) {
            Block *cur = requestCallOps[i]->getBlock();
            Block *prev = requestCallOps[i - 1]->getBlock();
            if (prev != cur) {
                prevEntMap.try_emplace(cur, prev);
            }
        }

        // Build the set of QC (EPR-request) blocks for the static-scheduler guard below.
        // When useOnlineScheduler is false, these blocks are exempt from the
        // floater-to-branch propagation: the direct memory-effect deps
        // (correction_block -> epr_block) already provide the correct ordering
        // for the static scheduler, and forcing them before every branch block
        // would prevent EPR generation from being pipelined with corrections.
        std::unordered_set<Block *> qcBlocks;
        if (!useOnlineScheduler) {
            for (Operation *op : requestCallOps) {
                qcBlocks.insert(op->getBlock());
            }
        }

        // Collect all conditional branch blocks (to avoid iterator invalidation
        // when we move blocks in Part 2).
        std::vector<Block *> condBrBlocks;
        for (Block &block : mainFunc) {
            Operation *term = block.getTerminator();
            if (term && isa<cf::CondBranchOp>(term)) {
                condBrBlocks.push_back(&block);
            }
        }

        for (Block *C : condBrBlocks) {
            // Iterate until no more moves are possible (moving one floater may
            // unblock another whose constraint pointed at the first).
            bool moved = true;
            while (moved) {
                moved = false;

                // Recompute block positions (may have changed from a prior move).
                blockPos.clear();
                pos = 0;
                for (Block &b : mainFunc) {
                    blockPos[&b] = pos++;
                }
                const uint32_t posC = blockPos[C];

                // Compute transitive data dependencies of ALL blocks after C.
                std::unordered_set<Block *> transitiveNeeds;
                std::vector<Block *> worklist;
                for (auto &[blk, blkP] : blockPos) {
                    if (blkP > posC) {
                        for (Block *dep : blockDeps[blk]) {
                            if (transitiveNeeds.insert(dep).second) {
                                worklist.push_back(dep);
                            }
                        }
                    }
                }
                while (!worklist.empty()) {
                    Block *cur = worklist.back();
                    worklist.pop_back();
                    for (Block *dep : blockDeps[cur]) {
                        if (transitiveNeeds.insert(dep).second) {
                            worklist.push_back(dep);
                        }
                    }
                }

                // Process each floater in the transitive needs set.
                for (Block *F : transitiveNeeds) {
                    if (F == C) {
                        continue;
                    }
                    // Skip blocks with CFG predecessors — they are part of a
                    // conditional structure and must not be moved.
                    if (F->pred_begin() != F->pred_end()) {
                        continue;
                    }
                    // Skip bridge blocks — they acquire synthesised predecessors
                    // from getPrevNode() and must not be moved.
                    if (isPureCfBridgeBlock(*F)) {
                        continue;
                    }

                    const uint32_t posF = blockPos[F];
                    if (posF < posC) {
                        // Part 1: F is already before C — add as dependency.
                        // For the static scheduler, skip EPR-request (QC) floaters:
                        // the direct memory-effect deps (correction_block -> epr_block)
                        // are sufficient, and this conservative constraint would
                        // prevent the MILP from pipelining EPR generation with corrections.
                        if (!useOnlineScheduler && qcBlocks.count(F)) {
                            continue;
                        }
                        if (blockDeps[C].insert(F).second) {
                            LLVM_DEBUG(llvm::dbgs() << "[FloaterProp] " << blockIdMap[C] << " now depends on "
                                                    << blockIdMap[F] << " (floater before branch)\n");
                        }
                    } else {
                        // Part 2: F is after C — check if all constraints of F
                        // are satisfiable before C, then move + add dep.
                        // For the static scheduler, also skip EPR-request (QC) floaters
                        // here to prevent them from being moved before branch blocks.
                        if (!useOnlineScheduler && qcBlocks.count(F)) {
                            continue;
                        }
                        bool canMove = true;
                        for (Block *dep : blockDeps[F]) {
                            if (blockPos[dep] >= posC) {
                                canMove = false;
                                break;
                            }
                        }
                        if (auto it = prevCommMap.find(F); it != prevCommMap.end() && blockPos[it->second] >= posC) {
                            canMove = false;
                        }
                        if (auto it = prevEntMap.find(F); it != prevEntMap.end() && blockPos[it->second] >= posC) {
                            canMove = false;
                        }
                        if (canMove) {
                            LLVM_DEBUG(llvm::dbgs() << "[FloaterProp] Moving " << blockIdMap[F] << " before "
                                                    << blockIdMap[C] << " and adding as dependency\n");
                            F->moveBefore(C);
                            blockDeps[C].insert(F);
                            moved = true;
                            break; // restart — positions changed
                        }
                    }
                }
            }
        }

        transitiveReduction(blockIdMap, blockDeps);

        // Compute the precedences data and add it to the block
        for (Block &block : mainFunc) {
            OpBuilder builder = OpBuilder::atBlockBegin(&block);

            auto blockIdAttr = builder.getStringAttr(blockIdMap[&block]);

            // Collect and sort (for determinism) data dependencies
            std::vector<StringRef> dataDepsId;
            for (Block *pred : blockDeps[&block]) {
                dataDepsId.emplace_back(blockIdMap[pred]);
            }
            std::sort(dataDepsId.begin(), dataDepsId.end());
            auto dataDepsAttr = builder.getStrArrayAttr(dataDepsId);

            // Control flow predecessors.
            //
            // A CFG predecessor edge A → B is pruned when all four conditions hold:
            //   1. B is a quantum block (QL or QC) — identified by the presence of a
            //      quantum subroutine call, i.e. an entry in callSiteEffects.
            //      We restrict pruning to quantum blocks to avoid creating many new
            //      independent classical blocks, which would balloon the number of MILP
            //      FCFS binary variables and make the solver intractable.
            //   2. B is a merge point (2+ CFG predecessors), guaranteeing B always
            //      executes regardless of which branch was taken.  Single-predecessor
            //      blocks are taken-branch targets that must keep their predecessor.
            //   3. A is not in blockDeps[B] — no SSA value defined by A is consumed
            //      by B, and the memory-effect analysis added no conflict dep.
            //   4. A and B have no conflicting callee memory effects on a shared
            //      quantum resource (same Value with at least one Write).
            //
            // The motivating case: an inter-qubit gate block (QL, merge point) whose
            // CFG predecessors are classical correction-branch blocks.  Those branches
            // contain no quantum operations and produce no values consumed by the gate,
            // so the predecessor edges are pure control-flow artefacts from the
            // sequential compilation order.  Dropping them lets the MILP hoist the
            // gate before the later correction phases, enabling streaming "measure-
            // as-you-go" schedules where earlier-allocated qubits are measured before
            // later EPR rounds are generated.
            const int numPreds = static_cast<int>(std::distance(block.pred_begin(), block.pred_end()));
            const bool isMergePoint = numPreds >= 2;
            const bool isQuantumBlock = callSiteEffects.count(&block) > 0;

            std::vector<StringRef> predsIds;
            for (Block *pred : block.getPredecessors()) {
                if (isMergePoint && isQuantumBlock) {
                    // Condition 3: pred must be a genuine data dependency.
                    if (!blockDeps[&block].count(pred)) {
                        // Condition 4: no quantum memory conflict between pred and block.
                        bool hasConflict = false;
                        if (callSiteEffects.contains(pred) && callSiteEffects.contains(&block)) {
                            for (const auto &effA : callSiteEffects.at(pred).effects) {
                                for (const auto &effB : callSiteEffects.at(&block).effects) {
                                    if (!effA.getValue() || !effB.getValue()) {
                                        continue;
                                    }
                                    if (effA.getValue() != effB.getValue()) {
                                        continue;
                                    }
                                    if (isa<MemoryEffects::Write>(effA.getEffect()) ||
                                        isa<MemoryEffects::Write>(effB.getEffect())) {
                                        hasConflict = true;
                                        break;
                                    }
                                }
                                if (hasConflict) {
                                    break;
                                }
                            }
                        }
                        if (!hasConflict) {
                            LLVM_DEBUG(llvm::dbgs()
                                       << "[PredPrune] Dropping pure-CF predecessor " << blockIdMap[pred] << " → "
                                       << blockIdMap[&block] << " (quantum merge-point, no data/memory dep)\n");
                            continue;
                        }
                    }
                }
                predsIds.emplace_back(blockIdMap[pred]);
            }
            // Synthesize a predecessor for orphan "bridge" blocks.
            if (predsIds.empty() && isPureCfBridgeBlock(block)) {
                if (Block *prev = block.getPrevNode()) {
                    predsIds.emplace_back(blockIdMap[prev]);
                }
            }
            std::sort(predsIds.begin(), predsIds.end());
            auto predsAttr = builder.getStrArrayAttr(predsIds);

            // Previous communication
            StringAttr prevCommAttr = builder.getStringAttr("");
            for (size_t i = 1; i < commOps.size(); i++) {
                if (commOps[i]->getBlock() == &block) {
                    Block *prevBlock = commOps[i - 1]->getBlock();
                    if (prevBlock != &block) {
                        prevCommAttr = builder.getStringAttr(blockIdMap[prevBlock]);
                    }
                    break; // only first match
                }
            }

            // Previous request call
            StringAttr prevReqAttr = builder.getStringAttr("");
            for (size_t i = 1; i < requestCallOps.size(); ++i) {
                if (requestCallOps[i]->getBlock() == &block) {
                    Block *prevBlock = requestCallOps[i - 1]->getBlock();
                    if (prevBlock != &block) {
                        prevReqAttr = builder.getStringAttr(blockIdMap[prevBlock]);
                    }
                    break;
                }
            }

            // In the meantime, we don't have deadlines information
            DictionaryAttr deadlinesAttr = DictionaryAttr::get(builder.getContext());

            // No existing BlkMeta, create a new one
            builder.create<qoalahost::BlkMeta>(block.front().getLoc(), blockIdAttr, predsAttr, dataDepsAttr,
                                               prevCommAttr, prevReqAttr, deadlinesAttr);

            LLVM_DEBUG(llvm::dbgs() << "Inserted new BlkMeta in " << blockIdMap[&block] << " with predecessors "
                                    << predsAttr << " with dependencies " << dataDepsAttr << " with previous comm "
                                    << prevCommAttr << " with previous ent " << prevReqAttr << "\n");
        }

        return success();
    }
} // namespace qoala::analysis::precedences
