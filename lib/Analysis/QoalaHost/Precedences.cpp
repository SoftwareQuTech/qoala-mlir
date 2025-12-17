#include <unordered_set>
#include <vector>

#include "Analysis/Helpers/Helpers.h"
#include "Analysis/NetQASM/Helpers.h"
#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "llvm/Support/Debug.h"
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

    LogicalResult addPrecedences(ModuleOp &moduleOp) {
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
        });

        resolveMemorySideEffects(callSiteEffects, blockIdMap, blockDeps);

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

            // Control flow predecessors
            std::vector<StringRef> predsIds;
            for (Block *pred : block.getPredecessors()) {
                predsIds.emplace_back(blockIdMap[pred]);
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
