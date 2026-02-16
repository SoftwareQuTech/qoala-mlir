#include "Analysis/NetQASM/Helpers.h"
#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/Helpers/DialectHelpers.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "Dialect/QoalaHost/Passes.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Tools/QoalaOpt.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormatVariadic.h"

#include "scip/scip.h"
#include "scip/scipdefplugins.h"

#define DEBUG_TYPE "qoalahost-reorder-blocks-pass-internal"

using namespace mlir;
using namespace qoala::dialects;
using namespace qoala::analysis;
using namespace qoala::options;

namespace qoala::analysis::reordering {
    inline Operation *resolveCallee(qoalahost::CallOp callOp, const llvm::StringMap<Operation *> &routineMap) {
        // Fast lookup of the callee routine for a qoalahost.call.
        // Returns nullptr if the symbol is not in `routineMap`.
        // `getCallee()` is cheaper than going through SymbolRefAttr plumbing.
        const StringRef symName = callOp.getCallee();
        return routineMap.contains(symName) ? routineMap.at(callOp.getCallee()) : nullptr;
    }

    static LogicalResult createTasksForBlock(MILPBlock *blk, const Location &loc) {
        // Creates the set of MILP tasks associated with a given block.

        const std::vector<std::unique_ptr<MILPOperation>> &ops = blk->getOperations();
        if (ops.empty()) {
            emitError(loc) << "MILPBlock '" << blk->getId() << "' has no operations — this should not happen.";
            return failure();
        }

        switch (blk->getType()) {
            case BlockType::CC:
                if (ops.size() != 1) {
                    emitError(loc) << "MILPBlock of type CC should contain exactly one operation, but got "
                                   << ops.size();
                    return failure();
                }
                break;
            case BlockType::CL: {
                auto task = std::make_unique<MILPTask>("0", blk, TaskGroup::C);
                for (const std::unique_ptr<MILPOperation> &op : ops) {
                    task->addOperation(op.get());
                }

                blk->addTask(std::move(task));
                break;
            }
            case BlockType::QL:
            case BlockType::QC: {
                if (ops.size() < 3) {
                    emitError(loc) << "QL/QC block must contain at least 3 operations to form 3 tasks";
                    return failure();
                }
                // Task 0 – call (C): PreTask
                auto t0 = std::make_unique<MILPTask>("0", blk, TaskGroup::C);
                t0->addOperation(ops.front().get());
                blk->addTask(std::move(t0));
                // Task 1 – middle (Q): Quantum Routine
                auto t1 = std::make_unique<MILPTask>("1", blk, TaskGroup::Q);
                for (size_t i = 1; i + 1 < ops.size(); ++i) {
                    t1->addOperation(ops[i].get());
                }
                blk->addTask(std::move(t1));
                // Task 2 – return (C) : PostTask.
                auto t2 = std::make_unique<MILPTask>("2", blk, TaskGroup::C);
                t2->addOperation(ops.back().get());
                blk->addTask(std::move(t2));
                break;
            }
        }
        return success();
    }

    static LogicalResult inlineCallIntoBlock(qoalahost::CallOp callOp, const std::string &blkId, uint32_t &opIdx,
                                             const llvm::StringMap<Operation *> &routineMap, Block *callerBlock,
                                             MILPBlock *blk,
                                             std::unordered_map<Operation *, MILPOperation *> &opToMilpOp) {
        // Inline the body of a qoalahost.call whose callee is a LocalRoutineOp or
        // RequestRoutineOp.  Appends the inlined operations (and the synthetic
        // qoalahost.nop) to `blk`, updates `opIdx`, and fills `opToMilpOp`.
        // Returns failure() if anything is malformed (e.g. callee not found,
        // callee does not end in netqasm.return).

        // Resolve the callee.
        Operation *callee = resolveCallee(callOp, routineMap);
        if (!callee || !isa<FunctionOpInterface>(callee)) {
            return callOp.emitError("Callee is not a FunctionOpInterface"), failure();
        }
        const auto calleeFunc = llvm::cast<FunctionOpInterface>(callee);

        bool foundReturn = false;
        bool foundOpWithoutDuration = false;

        // Inline every op from the callee
        calleeFunc->walk([&](Operation *innerOp) -> WalkResult {
            // Create MILPOperation for every inlined op
            std::string subId = blkId + "::" + std::to_string(opIdx++);
            if (auto durationOp = dyn_cast<helpers::QuantumOpInterface>(innerOp)) {
                auto milpSub = std::make_unique<MILPOperation>(subId, durationOp.getDuration());
                milpSub->setOperation(innerOp);
                MILPOperation *raw = blk->addOperation(std::move(milpSub));
                opToMilpOp[innerOp] = raw;

                // When we hit the netqasm.return, insert the synthetic nop
                if (llvm::isa<dialects::netqasm::ReturnOp>(innerOp)) {
                    OpBuilder builder(callOp.getContext());
                    builder.setInsertionPointToEnd(callerBlock);
                    auto nop = builder.create<qoalahost::NopOp>(innerOp->getLoc());

                    std::string nopId = blkId + "::" + std::to_string(opIdx++);
                    auto milpNop = std::make_unique<MILPOperation>(nopId, nop.getDuration());
                    milpNop->setOperation(nop.getOperation());
                    MILPOperation *rawNop = blk->addOperation(std::move(milpNop));
                    opToMilpOp[nop.getOperation()] = rawNop;

                    foundReturn = true;
                    return WalkResult::interrupt(); // stop the walk
                }
                return WalkResult::advance();
            }
            foundOpWithoutDuration = true;
            return WalkResult::interrupt();
        });

        if (foundOpWithoutDuration) {
            return callOp.emitError("Operation inside a callee without known duration"), failure();
        }

        if (!foundReturn) {
            return callOp.emitError("Callee does not end in netqasm::ReturnOp"), failure();
        }

        return success();
    }

    static void recordEdge(const StringRef predId, const llvm::StringMap<MILPBlock *> &idToBlockMap,
                           std::vector<std::pair<MILPBlock *, MILPBlock *>> &precedences,
                           std::vector<std::pair<std::string, std::string>> &unresolvedEdges, MILPBlock *blk,
                           const std::string &blkId) {
        if (predId.empty()) {
            return;
        }

        if (idToBlockMap.contains(predId)) {
            precedences.emplace_back(idToBlockMap.at(predId), blk);
        } else {
            unresolvedEdges.emplace_back(predId.str(), blkId);
        }
    }

    llvm::StringMap<Operation *> collectRoutineMap(ModuleOp &moduleOp) {
        llvm::StringMap<Operation *> routineMap;

        moduleOp.walk([&](helpers::NetQASMRoutineInterface routine) {
            Operation *op = routine.getOperation();

            if (auto localRoutine = llvm::dyn_cast<dialects::netqasm::LocalRoutineOp>(op)) {
                routineMap.try_emplace(localRoutine.getSymName(), op);
            } else if (auto requestRoutine = llvm::dyn_cast<dialects::netqasm::RequestRoutineOp>(op)) {
                routineMap.try_emplace(requestRoutine.getSymName(), op);
            }
        });

        return routineMap;
    }

    std::tuple<std::vector<std::shared_ptr<MILPBlock>>, std::unordered_map<Operation *, MILPOperation *>,
               BlockPrecedenceList, std::vector<std::pair<std::string, std::string>>, llvm::StringMap<MILPBlock *>,
               LogicalResult>
    buildMilpBlocks(qoalahost::MainFuncOp &mainFunc, const llvm::StringMap<Operation *> &routineMap) {
        std::vector<std::shared_ptr<MILPBlock>> blocks;
        BlockPrecedenceList precedences;

        llvm::StringMap<MILPBlock *> idToBlockMap;
        std::unordered_map<Operation *, MILPOperation *> opToMilpOp;
        std::vector<std::pair<std::string, std::string>> unresolvedEdges;

        llvm::DenseSet<Block *> visitedBlocks;
        LogicalResult status = success();

        // Prepass: compute incoming-reference counts and whether a block has outgoing meta refs
        struct MetaRefInfo {
            bool hasOutgoing = false; // this block's blk_meta has any deps/preds/prev_* set
            int incoming = 0; // how many other blocks reference this block_id
        };

        llvm::StringMap<MetaRefInfo> metaRefInfo;
        llvm::StringMap<int> incomingCountTmp;

        // Only count each MLIR block once.
        llvm::DenseSet<Block *> seenForMetaPrepass;

        auto bumpIncoming = [&](StringRef target) {
            if (!target.empty()) {
                incomingCountTmp[target] += 1;
            }
        };

        mainFunc.walk([&](qoalahost::BlkMeta blkMeta) -> WalkResult {
            Block *b = blkMeta->getBlock();
            if (!seenForMetaPrepass.insert(b).second) {
                return WalkResult::advance();
            }

            StringRef id = blkMeta.getBlockId();
            MetaRefInfo &info = metaRefInfo[id]; // ensure exists

            // Outgoing refs present in this block's metadata?
            bool hasPreds = false;
            if (const ArrayAttr a = blkMeta.getPredecessorsAttr()) {
                hasPreds = !a.empty();
            }

            bool hasDeps = false;
            if (const ArrayAttr a = blkMeta.getDependenciesAttr()) {
                hasDeps = !a.empty();
            }

            bool hasPrevEnt = false;
            if (const StringAttr a = blkMeta.getPrevEntAttr()) {
                hasPrevEnt = !a.getValue().empty();
            }

            bool hasPrevComm = false;
            if (const StringAttr a = blkMeta.getPrevCommAttr()) {
                hasPrevComm = !a.getValue().empty();
            }

            info.hasOutgoing = hasPreds || hasDeps || hasPrevEnt || hasPrevComm;

            // Record incoming refs: who does this blk_meta mention?
            if (const ArrayAttr a = blkMeta.getPredecessorsAttr()) {
                for (const StringRef s : a.getAsValueRange<StringAttr>()) {
                    bumpIncoming(s);
                }
            }
            if (const ArrayAttr a = blkMeta.getDependenciesAttr()) {
                for (const StringRef s : a.getAsValueRange<StringAttr>()) {
                    bumpIncoming(s);
                }
            }
            if (const StringAttr a = blkMeta.getPrevEntAttr()) {
                bumpIncoming(a.getValue());
            }
            if (const StringAttr a = blkMeta.getPrevCommAttr()) {
                bumpIncoming(a.getValue());
            }

            return WalkResult::advance();
        });

        // Finalize incoming counts.
        for (auto &kv : metaRefInfo) {
            kv.second.incoming = incomingCountTmp.lookup(kv.first());
        }

        // Helper: decide if we should skip an empty block entirely.
        auto isIsolatedEmptyBlock = [&](StringRef blkId) -> bool {
            auto it = metaRefInfo.find(blkId);
            if (it == metaRefInfo.end()) {
                return false; // be conservative
            }
            return !it->second.hasOutgoing && it->second.incoming == 0;
        };
        // End prepass

        // This walk processes each `qoalahost.BlkMeta` operation in the `mainFunc`
        // body exactly once (per MLIR block) to construct the high-level MILP structure.
        mainFunc.walk([&](qoalahost::BlkMeta blkMeta) -> WalkResult {
            if (failed(status)) {
                return WalkResult::interrupt();
            }

            Block *block = blkMeta->getBlock();
            if (!visitedBlocks.insert(block).second) {
                return WalkResult::advance();
            }

            // Avoid processing the same MLIR block twice (only one BlkMeta per block is valid)
            if (block->empty()) {
                emitError(blkMeta.getLoc(), "Block has no body after BlkMeta");
                status = failure();
                return WalkResult::interrupt();
            }

            // Skip this block entirely if it contains a qoalahost::ReturnOp.
            // Even if the ReturnOp is not the last operation, we ignore the entire block
            // because:
            //   - ReturnOps only appear in classical (CL) blocks
            //   - These blocks typically have minimal performance impact
            //   - Optimizing their placement is unlikely to affect overall program performance
            // Thus, any CL block containing a ReturnOp is excluded from MILP modeling.
            if (!block->getOps<qoalahost::ReturnOp>().empty()) {
                LLVM_DEBUG(llvm::dbgs() << "Skipping block with ReturnOp: " << blkMeta.getBlockId() << "\n");
                return WalkResult::advance();
            }
            // If entanglement requests are grouped (qoalaOptGroupEntReqs = true),
            // we require all blocks that perform entanglement (i.e., netqasm.request_routine)
            // to execute at the start of the program. These blocks are excluded from MILP modeling
            // to prevent reordering. Here, we skip any such block (QC type) during MILP block creation.
            if (qoalaOptGroupEntReqs && block->getOperations().size() > 1) {
                const auto *firstNonBlkMetaOp = block->begin()->getNextNode();

                if (auto iface = dyn_cast<helpers::QuantumOpInterface>(firstNonBlkMetaOp)) {
                    if (iface.getBlockType(routineMap) == BlockType::QC) {
                        LLVM_DEBUG(llvm::dbgs() << "Skipping entanglement block (QC) due to grouping: "
                                                << blkMeta.getBlockId() << "\n");
                        return WalkResult::advance();
                    }
                }
            }

            const auto firstIt = std::next(block->begin());
            Operation *firstNonBlkMetaOp = &*firstIt;
            LLVM_DEBUG(llvm::dbgs() << *firstNonBlkMetaOp << "\n");
            BlockType blkType;
            if (auto ifaceFirstOp = dyn_cast<helpers::QuantumOpInterface>(firstNonBlkMetaOp)) {
                blkType = ifaceFirstOp.getBlockType(routineMap);
            } else {
                // This was a weird behavior; if the operation cannot be casted (e.g. null),
                // we assume CL type
                blkType = BlockType::CL;
            }

            // Create new MILPBlock object and associate with block ID and type
            std::string blkId = blkMeta.getBlockId().str();
            auto blkPtr = std::make_shared<MILPBlock>(blkId, blkType);
            blkPtr->setBlock(block);
            MILPBlock *blk = blkPtr.get();
            idToBlockMap[blkId] = blk;

            // Index operations to create unique IDs for MILPOperation
            uint32_t opIdx = 0;

            // Walk through the actual instructions in the MLIR block
            for (Operation &op : *block) {
                if (isa<qoalahost::BlkMeta>(op)) {
                    continue; // Skip BlkMeta
                }

                // Stop early if we reach a return/nop_term in a classical block
                if (blkType == BlockType::CL && llvm::isa<qoalahost::NopTOp>(op)) {
                    break;
                }

                // Create MILPOperation
                std::string opId = blkId + "::" + std::to_string(opIdx++);

                // Decide duration:
                // - QuantumOpInterface ops use their interface duration
                // - Non-interface ops in CL blocks use host-instruction time
                // - Non-interface ops in non-CL blocks are ignored
                int64_t dur = 0;
                bool shouldAdd = false;

                if (auto durationOp = dyn_cast<helpers::QuantumOpInterface>(&op)) {
                    dur = durationOp.getDuration();
                    shouldAdd = true;
                } else if (blkType == BlockType::CL) {
                    dur = qoalaOptHostInstrTime;
                    shouldAdd = true;
                }

                if (shouldAdd) {
                    auto milpOp = std::make_unique<MILPOperation>(opId, dur);
                    milpOp->setOperation(&op);
                    MILPOperation *raw = blk->addOperation(std::move(milpOp));
                    opToMilpOp[&op] = raw;

                    // Handle inlining for quantum-local or quantum-communication blocks
                    // - Inline body of the called routine
                    // - Track operations
                    // - Add a qoalahost.nop at the end to model post-task transition
                    if (blkType == BlockType::QL || blkType == BlockType::QC) {
                        if (const auto callOp = llvm::dyn_cast<qoalahost::CallOp>(op)) {
                            if (failed(inlineCallIntoBlock(callOp, blkId, opIdx, routineMap,
                                                           block, // callerBlock
                                                           blk, opToMilpOp))) {
                                status = failure();
                                return WalkResult::interrupt();
                            }
                            break; // only one call per block
                        }
                    }
                }

                // CC blocks contain a single communication operation only
                if (blkType == BlockType::CC) {
                    break;
                }
            }

            if (failed(status)) {
                return WalkResult::interrupt();
            }

            // If this block produced no MILP operations, it might be an "empty" block:
            // e.g., only blk_meta + nop_term. If it's also isolated in blk_meta (no deps/preds/prev_*)
            // AND nobody references it, then skip it entirely (do not include in MILP).
            if (blk->getOperations().empty() && isIsolatedEmptyBlock(blkId)) {
                LLVM_DEBUG(llvm::dbgs() << "Skipping isolated empty block: " << blkId << "\n");
                idToBlockMap.erase(blkId);
                return WalkResult::advance();
            }

            // Record precedence edges from block attributes. For our optimization all types of precedences are
            // equivalent. The differenciation is needed for the translation step.
            if (const ArrayAttr a = blkMeta.getPredecessorsAttr()) {
                for (const StringRef s : a.getAsValueRange<StringAttr>()) {
                    recordEdge(s, idToBlockMap, precedences, unresolvedEdges, blk, blkId);
                }
            }
            if (const ArrayAttr a = blkMeta.getDependenciesAttr()) {
                for (const StringRef s : a.getAsValueRange<StringAttr>()) {
                    recordEdge(s, idToBlockMap, precedences, unresolvedEdges, blk, blkId);
                }
            }
            if (const StringAttr a = blkMeta.getPrevEntAttr()) {
                recordEdge(a.getValue(), idToBlockMap, precedences, unresolvedEdges, blk, blkId);
            }
            if (const StringAttr a = blkMeta.getPrevCommAttr()) {
                recordEdge(a.getValue(), idToBlockMap, precedences, unresolvedEdges, blk, blkId);
            }

            // Generate tasks for this MILP block (task-level subdivision)
            if (failed(createTasksForBlock(blk, blkMeta.getLoc()))) {
                status = failure();
                return WalkResult::interrupt();
            }

            blocks.push_back(std::move(blkPtr));
            return WalkResult::advance();
        });

        return {blocks, opToMilpOp, precedences, unresolvedEdges, idToBlockMap, status};
    }

    std::tuple<llvm::DenseMap<Value, std::vector<Operation *>>, LogicalResult>
    collectQubitUsage(qoalahost::MainFuncOp &mainFunc, ModuleOp &moduleOp) {
        // Maps canonicalized Qubit Value to list of ops using it (e.g., qinit, measure, epr)
        llvm::DenseMap<Value, std::vector<Operation *>> qubitToOps;
        // Maps result of call to actual QAlloc op it aliases (transitive resolution)
        llvm::DenseMap<Value, Value> resolvedQubitAlias;

        LogicalResult status = success();

        // Helper to resolve the final canonical Qubit value (transitive alias flattening)
        auto resolve = [&](Value v) -> Value {
            SmallPtrSet<Value, 4> seen;
            while (resolvedQubitAlias.count(v)) {
                if (!seen.insert(v).second) {
                    break;
                }
                v = resolvedQubitAlias[v];
            }
            return v;
        };

        // Walk all CallOps, analyze their callee routines
        //  - Collect MemoryEffect ops inside the inlined body
        //  - Track any returned qubit (e.g. %0 = call @foo) that aliases a QAlloc
        mainFunc.walk([&](qoalahost::CallOp callOp) -> WalkResult {
            // const auto symRef = callOp.getCalleeAttr().dyn_cast_or_null<SymbolRefAttr>();
            // if (!symRef) {
            //     return WalkResult::advance();
            // }
            //
            // // Resolve callee definition from symbol table
            // Operation *callee = SymbolTable::lookupNearestSymbolFrom(moduleOp, symRef);
            // if (!callee || callee->getNumRegions() == 0) {
            //     return WalkResult::advance();
            // }

            Operation *callee = dialects::helpers::getRoutineWithName(&moduleOp, callOp.getCallee());
            Block &entry = callee->getRegion(0).front();

            // Map formal function arguments to actual call operands
            llvm::DenseMap<Value, Value> argMap;
            const ValueRange formalArgs = entry.getArguments();
            const ValueRange actualArgs = callOp->getOperands();
            for (size_t i = 0, e = std::min(formalArgs.size(), actualArgs.size()); i < e; ++i) {
                argMap[formalArgs[i]] = actualArgs[i];
            }
            // netqasm::ArgValueMap argMap = netqasm::getRoutineArgValues(callee, callOp.getOperands());

            // Traverse the callee body and collect MemoryEffect ops (like qinit, epr, measure)
            callee->walk([&](MemoryEffectOpInterface memOp) {
                SmallVector<MemoryEffects::EffectInstance, 4> effects;
                memOp.getEffects(effects);

                for (MemoryEffects::EffectInstance &eff : effects) {
                    if (Value v = eff.getValue()) {
                        // Resolve value through argument mapping
                        Value resolved = argMap.lookup(v);
                        if (!resolved) {
                            resolved = v;
                        }

                        // Canonicalize through aliases (if applicable)
                        Value canonical = resolve(resolved);
                        qubitToOps[canonical].push_back(memOp.getOperation());

                        LLVM_DEBUG(llvm::dbgs() << "    [Track] " << memOp->getName() << " → " << canonical << '\n');
                    }
                }
            });

            // If call has no result, skip alias tracking
            if (callOp->getNumResults() == 0) {
                return WalkResult::advance();
            }

            // Try to associate call results with returned QAlloc values
            bool foundReturn = false;
            callee->walk([&](dialects::netqasm::ReturnOp ret) {
                const ValueRange retVals = ret.getOperands();
                const auto callResults = callOp->getResults();

                for (size_t i = 0, e = std::min(retVals.size(), callResults.size()); i < e; ++i) {
                    const Value &result = callResults[i];
                    const Value retVal = retVals[i];
                    const Value final = resolve(retVal);

                    // If the returned value comes from a QAlloc, record the alias
                    if (auto *def = final.getDefiningOp(); def && isa<dialects::netqasm::QAllocOp>(def)) {
                        resolvedQubitAlias[result] = final;
                        LLVM_DEBUG(llvm::dbgs() << "  [Alias→QAlloc] " << result << " ↦ " << final << " (qalloc)\n");
                    } else {
                        LLVM_DEBUG(llvm::dbgs() << "  [Skip Alias] " << result << " ↦ " << final << " (not qalloc)\n");
                    }
                }

                foundReturn = true;
                return WalkResult::interrupt(); // stop after first ReturnOp
            });

            if (!foundReturn) {
                status = failure();
                emitError(callOp.getLoc(), "callee has no netqasm.return instruction");
                return WalkResult::interrupt();
            }

            return WalkResult::advance();
        });

        return {qubitToOps, status};
    }

    static std::vector<std::shared_ptr<MILPQubit>>
    buildMilpQubits(const llvm::DenseMap<Value, std::vector<Operation *>> &qubitToOps,
                    const std::unordered_map<Operation *, MILPOperation *> &opToMilpOp) {
        std::vector<std::shared_ptr<MILPQubit>> qubits;

        uint32_t qubitIndex = 0;

        // Construct MILPQubit objects
        // - Extract alloc & meas ops from qubit usage
        // - Create MILPQubit and attach relevant operations
        for (const auto &[qubitVal, ops] : qubitToOps) {
            // const std::vector<Operation *> &ops = entry.second;

            std::string id = "q" + std::to_string(qubitIndex++);
            auto qubitPtr = std::make_shared<MILPQubit>(id);

            MILPOperation *allocOp = nullptr;
            MILPOperation *measOp = nullptr;

            for (Operation *op : ops) {
                if (llvm::isa<dialects::netqasm::QInitOp>(op) || llvm::isa<dialects::netqasm::EprsOp>(op)) {
                    auto it = opToMilpOp.find(op);
                    allocOp = (it != opToMilpOp.end()) ? it->second : nullptr;
                }

                if (llvm::isa<dialects::netqasm::MeasureOp>(op)) {
                    auto itMeas = opToMilpOp.find(op);
                    measOp = (itMeas != opToMilpOp.end()) ? itMeas->second : nullptr;
                }
            }

            // Attach known alloc/meas to the qubit model object
            if (allocOp) {
                qubitPtr->setAllocation(allocOp);
            }
            if (measOp) {
                qubitPtr->setMeasurement(measOp);
            }

            qubits.push_back(std::move(qubitPtr));
        }

        return qubits;
    }

    std::tuple<std::vector<std::shared_ptr<MILPBlock>>, std::vector<std::shared_ptr<MILPQubit>>, BlockPrecedenceList,
               llvm::StringMap<MILPBlock *>, LogicalResult>
    buildMILPFromMLIR(ModuleOp &moduleOp) {
        // Constructs an intermediate MILP model from a given QoalaHost IR module.
        // This function performs a multi-pass traversal over the input MLIR module to
        // extract high-level scheduling constraints in preparation for MILP solving.
        // The steps are as follows:
        // 1. Walk the main function (`MainFuncOp`) and extract every `qoalahost.BlkMeta`.
        //    For each such block:
        //      - Create a MILPBlock instance and assign it a unique ID (taken from BlkMeta).
        //      - Collect all operations (including inlined subroutines) as MILPOperations.
        //      - Classify the block's type (CL, CC, QL, QC) using its first non-BlkMeta op.
        //      - Create Tasks within each block.
        // 2. For each block and operation:
        //      - Inline the body of `qoalahost.call` instructions if they target `LocalRoutineOp` or
        //      `RequestRoutineOp`, preserving temporal structure.
        //      - Handle `netqasm.return` instructions in inlined routines by inserting a `qoalahost.nop` operation
        //      in the caller block to explicitly represent a PostTask from the qoala runtime model.
        //      - Track static dependencies declared in BlkMeta (predecessors, dependencies, prevComm and prevEnt).
        // 3. Resolve operation-to-qubit associations:
        //      - Analyze memory effect interfaces inside inlined routines.
        //      - Use value-based alias tracking to resolve logical qubits across calls.
        //      - For each qubit, record its init (netqasm.init or netqasm.eprs) and measure operations.
        // The resulting data structures are then passed into MILPModelBuilder, which
        // builds SCIP variables and constraints based on this semantic input.
        // Failure is returned if invalid constructs are encountered (e.g., unknown block
        // types, malformed calls, or unresolved aliases).

        auto mainFuncs = moduleOp.getOps<qoalahost::MainFuncOp>();
        if (mainFuncs.empty()) {
            emitError(moduleOp.getLoc(), "No main function found in module");
            return {{}, {}, {}, {}, failure()};
        }
        qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();

        llvm::StringMap<Operation *> routineMap = collectRoutineMap(moduleOp);

        auto [blocks, opToMilpOp, precedences, unresolvedEdges, idToBlockMap, blocksStatus] =
                buildMilpBlocks(mainFunc, routineMap);
        if (failed(blocksStatus)) {
            return {{}, {}, {}, {}, failure()};
        }

        // Fail if unresolved edges remain
        if (!unresolvedEdges.empty()) {
            if (!qoalaOptGroupEntReqs) {
                emitError(moduleOp.getLoc(), "Unresolved precedence edges detected");
                return {{}, {}, {}, {}, failure()};
            }
            // If entanglement request blocks are grouped at the beginning (qoalaOptGroupEntReqs=true),
            // we intentionally skip including those blocks in the MILP model. As a result, any
            // precedence edges that reference those omitted blocks will appear unresolved here.
            // This is expected and not an error in this context. The Qoala verifier will later
            // ensure the overall IR consistency.
            emitWarning(moduleOp.getLoc(),
                        "Unresolved precedence edges detected. Might be because of entanglement grouping");
        }

        LLVM_DEBUG({
            llvm::dbgs() << "[MILP] Blocks and tasks:\n";
            for (const auto &bp : blocks) {
                const auto *b = bp.get();
                llvm::dbgs() << " - Block " << b->getId() << " (type=" << static_cast<int>(b->getType()) << ")\n";
                for (const auto &tPtr : b->getTasks()) {
                    const auto *t = tPtr.get();
                    llvm::dbgs() << "    * Task " << t->getId() << " [Group=" << static_cast<int>(t->getGroup())
                                 << "]\n";
                    for (const auto *op : t->getOperations()) {
                        llvm::dbgs() << "        - " << op->getId() << " => " << op->getOperation()->getName()
                                     << " , duration=" << op->getDuration() << "ns\n";
                    }
                }
            }
        });

        LLVM_DEBUG({
            llvm::dbgs() << "[MILP] Precedence edges:\n";
            for (const auto &[firstPred, secondPred] : precedences) {
                llvm::dbgs() << "  " << firstPred->getId() << " -> " << secondPred->getId() << "\n";
            }
        });

        LLVM_DEBUG(llvm::dbgs() << "=== Generating all MILPQubits ===\n");

        auto [qubitToOps, collStatus] = collectQubitUsage(mainFunc, moduleOp);
        if (failed(collStatus)) {
            return {{}, {}, {}, {}, failure()};
        }

        LLVM_DEBUG({
            llvm::dbgs() << "[Qubit→Ops]\n";
            for (auto &[qubitVal, ops] : qubitToOps) {
                llvm::dbgs() << " - Raw Qubit: " << qubitVal << "\n";
                for (auto *op : ops) {
                    llvm::dbgs() << "     * " << op->getName() << "\n";
                }
            }
        });

        std::vector<std::shared_ptr<MILPQubit>> qubits = buildMilpQubits(qubitToOps, opToMilpOp);

        LLVM_DEBUG({
            llvm::dbgs() << "[MILP] Constructed MILPQubits:\n";
            for (const auto &q : qubits) {
                llvm::dbgs() << " - " << q->getId() << ": alloc=";
                llvm::dbgs() << (q->getAllocation() ? q->getAllocation()->getId() : "null") << ", meas=";
                llvm::dbgs() << (q->getMeasurement() ? q->getMeasurement()->getId() : "null") << "\n";
            }
        });

        return {blocks, qubits, precedences, idToBlockMap, success()};
    }

    inline SCIP_VAR *createVariable(SCIP *scip, const std::string &name, const bool strictlyPositive) {
        const double lb = strictlyPositive ? 1.0 : 0.0;
        SCIP_VAR *v = nullptr;
        SCIPcreateVarBasic(scip, &v, name.c_str(), lb, SCIPinfinity(scip), 0.0, SCIP_VARTYPE_INTEGER);
        SCIPaddVar(scip, v);
        return v;
    }

    bool MILPModelBuilder::checkSolverStatus(ModuleOp *op) const {
        switch (const SCIP_STATUS status = SCIPgetStatus(scip_)) {
            case SCIP_STATUS_OPTIMAL:
                LLVM_DEBUG(llvm::dbgs() << "[SCIP] Optimal solution found.\n");
                return true;
            case SCIP_STATUS_INFEASIBLE:
                if (op) {
                    op->emitError("MILP infeasible — check constraints.");
                }
                return false;
            case SCIP_STATUS_UNBOUNDED:
                if (op) {
                    op->emitError("MILP problem is unbounded.");
                }
                return false;
            case SCIP_STATUS_UNKNOWN:
                if (op) {
                    op->emitError("MILP solver status is unknown.");
                }
                return false;
            default:
                if (op) {
                    op->emitError("MILP solver returned status code: " + Twine(status));
                }
                return false;
        }
    }

    bool MILPModelBuilder::initialize() {
        if (SCIPcreate(&scip_) != SCIP_OKAY) {
            return false;
        }
        if (SCIPincludeDefaultPlugins(scip_) != SCIP_OKAY) {
            return false;
        }

        // Silence SCIP output
        SCIPsetMessagehdlrQuiet(scip_, TRUE);

        // Determinism: single-thread + fixed seeds
        SCIPsetIntParam(scip_, "parallel/maxnthreads", 1);
        SCIPsetIntParam(scip_, "parallel/minnthreads", 1);
        SCIPsetIntParam(scip_, "lp/threads", 1);
        SCIPsetIntParam(scip_, "randomization/randomseedshift", 0);
        SCIPsetIntParam(scip_, "randomization/lpseed", 0);
        SCIPsetIntParam(scip_, "randomization/permutationseed", 0);

        // Tighten numerics a bit (optional but helps consistency)
        SCIPsetRealParam(scip_, "numerics/feastol", 1e-9);
        SCIPsetRealParam(scip_, "numerics/epsilon", 1e-9);

        // Disable automatic symmetry detection and exploitation.
        // Even with fixed seeds and single-threading, SCIP's symmetry handling can
        // permute equivalent subproblems when multiple symmetric optimal solutions
        // exist. Turning it off removes that nondeterminism at the cost of a slight
        // performance decrease but ensures bit-for-bit reproducible schedules.
        // NOTE: In some SCIP versions this param is Int (enum), in others Bool.
        // Try Int first (0 = off), fall back to Bool(FALSE).
        {
            SCIP_RETCODE rc = SCIPsetIntParam(scip_, "misc/usesymmetry", 0 /* off */);
            if (rc != SCIP_OKAY) {
                // Older/newer builds may expose it as Bool. Ignore rc here deliberately.
                (void) SCIPsetBoolParam(scip_, "misc/usesymmetry", FALSE);
            }
        }

        // Create the (still empty) problem before any variables are added.
        if (SCIPcreateProbBasic(scip_, "MILP_BlockOrdering") != SCIP_OKAY) {
            return false;
        }
        return true;
    }

    void MILPBlockOrderModel::createVariables() {
        uint32_t total = 0;
        for (const std::shared_ptr<MILPBlock> &blk : blocks_) {
            for (const std::unique_ptr<MILPOperation> &op : blk->getOperations()) {
                total += op->getDuration();
                const std::string &id = op->getId();
                if (!startVars_.count(id)) {
                    startVars_[id] = createVariable(scip_, "s_" + id, /*strictlyPositive=*/false);
                }
            }
        }
        bigM_ = 2 * total;
        LLVM_DEBUG(llvm::dbgs() << "M=" << bigM_ << "\n");

        // Bound the schedule horizon so SCIP's LP relaxation is tight.
        // bigM = 2 * total is generous enough to never cut off an optimal solution,
        // but prevents the solver from exploring unbounded start-time ranges
        // (especially important for the secondary objective).
        for (auto &kv : startVars_) {
            SCIPchgVarUb(scip_, kv.second, static_cast<SCIP_Real>(bigM_));
        }
    }

    void MILPBlockOrderModel::addIntraTaskOrderingConstraints() {
        // Adds equality constraints to enforce strict sequential execution of operations
        // within the same task (i.e., intra-task ordering).
        // For each pair of consecutive operations `o1`, `o2` in a task, we enforce:
        //      start(o2) = start(o1) + duration(o1)
        // This is encoded as a linear constraint:
        //      start(o2) - start(o1) = duration(o1)
        // Which ensures operations within a task run in a chain without overlap.

        for (const std::shared_ptr<MILPBlock> &blk : blocks_) {
            for (const std::unique_ptr<MILPTask> &t : blk->getTasks()) {
                const std::vector<MILPOperation *> &ops = t->getOperations();
                for (size_t j = 0; j + 1 < ops.size(); ++j) {
                    const MILPOperation *o1 = ops[j];
                    const MILPOperation *o2 = ops[j + 1];
                    SCIP_CONS *c;
                    std::string name = "ord_" + o1->getId() + "_" + o2->getId();
                    const int32_t rhs = o1->getDuration();
                    LLVM_DEBUG(llvm::dbgs() << "A...rhs = " << rhs << "\n");
                    SCIPcreateConsBasicLinear(scip_, &c, name.c_str(), 0, nullptr, nullptr, rhs, rhs);
                    SCIPaddCoefLinear(scip_, c, startVars_[o2->getId()], 1.0);
                    SCIPaddCoefLinear(scip_, c, startVars_[o1->getId()], -1.0);
                    SCIPaddCons(scip_, c);
                    SCIPreleaseCons(scip_, &c);
                }
            }
        }
    }

    void MILPBlockOrderModel::addBlockPrecedenceConstraints() {
        // Adds precedence constraints between blocks based on their dependency edges.
        // If block A must precede block B, we enforce:
        //      start(firstOp(B)) >= start(lastOp(A)) + duration(lastOp(A))
        // This ensures B's operations begin only after A's last operation finishes.
        // Implemented using a linear inequality with a lower bound (right-hand side).

        for (const auto &[pred, succ] : precedences_) {
            // const MILPBlock *pred = e.first;
            // const MILPBlock *succ = e.second;
            const MILPOperation *predLast = pred->lastOp();
            const MILPOperation *succFirst = succ->firstOp();

            SCIP_CONS *c;
            std::string name = "prec_" + pred->getId() + "_" + succ->getId();
            const int32_t lhs = predLast->getDuration();
            LLVM_DEBUG(llvm::dbgs() << "B...lhs = " << lhs << "\n");
            SCIPcreateConsBasicLinear(scip_, &c, name.c_str(), 0, nullptr, nullptr, lhs, SCIPinfinity(scip_));
            SCIPaddCoefLinear(scip_, c, startVars_[succFirst->getId()], 1.0);
            SCIPaddCoefLinear(scip_, c, startVars_[predLast->getId()], -1.0);
            SCIPaddCons(scip_, c);
            SCIPreleaseCons(scip_, &c);
        }
    }

    static bool reachable(const MILPBlock *a, const MILPBlock *b, const Closure &C) {
        // Determines whether there exists a transitive precedence path from block `a` to block `b`
        // based on the computed closure of the precedence graph.
        // This is used to identify whether two blocks are already ordered with respect to each other,
        // which is particularly relevant when enforcing FCFS constraints only between independent blocks.
        return C.count({a->getId(), b->getId()}) > 0;
    };

    void MILPBlockOrderModel::addFCFSTaskConstraints() {
        // Adds mutual exclusion constraints between *tasks* of different blocks that are not transitively ordered
        // by existing precedence constraints. These constraints enforce *First-Come-First-Served* (FCFS) ordering.
        // For each unordered pair of blocks (b1, b2) that:
        //   - are not reachable from one another (i.e., no precedence edge or transitive dependency),
        //   - belong to the same task group, and
        //   - both contain at least one task,
        // we introduce a binary selector variable `z`. This variable determines the relative execution order:
        //   - If z = 0 -> all relevant tasks in b1 must finish before the corresponding tasks in b2 can start.
        //   - If z = 1 -> the reverse: tasks in b2 must complete before b1 can begin those same task slots.
        // If both blocks are quantum (i.e., composed of 3 tasks), constraints are applied for each of the 3
        // corresponding task slots (e.g., task 0 in b1 must finish before task 0 in b2). If either block is classical
        // (i.e., has only a single task), the constraint is applied only on the first task.
        // These constraints are enforced using a standard *big-M* encoding to allow either order (but not both).

        const uint32_t eps = 1;

        // Build transitive closure of the precedence DAG (reachable pairs).
        Closure clos;
        for (const auto &[firstPred, secondPred] : precedences_) {
            clos.insert({firstPred->getId(), secondPred->getId()});
        }

        bool grown;
        do {
            grown = false;
            for (const std::pair<std::string, std::string> &p : clos) {
                for (const std::pair<std::string, std::string> &q : clos) {
                    if (p.second == q.first && clos.insert({p.first, q.second}).second) {
                        grown = true;
                    }
                }
            }
        } while (grown);

        // Enumerate unordered block pairs that share the same task group.
        const uint32_t B = static_cast<uint32_t>(blocks_.size());
        for (uint32_t i = 0; i < B; ++i) {
            const MILPBlock *b1 = blocks_[i].get();
            for (uint32_t j = i + 1; j < B; ++j) {
                const MILPBlock *b2 = blocks_[j].get();

                // Skip if already ordered, have no tasks, or different groups.
                if (reachable(b1, b2, clos) || reachable(b2, b1, clos)) {
                    continue;
                }
                if (b1->getTasks().empty() || b2->getTasks().empty()) {
                    continue;
                }
                if (b1->getTasks().front()->getGroup() != b2->getTasks().front()->getGroup()) {
                    continue;
                }

                // Binary selector z
                SCIP_VAR *z;
                std::string zname = "z_" + b1->getId() + "_" + b2->getId();
                SCIPcreateVarBasic(scip_, &z, zname.c_str(), 0.0, 1.0, 0.0, SCIP_VARTYPE_BINARY);
                SCIPaddVar(scip_, z);

                // Which task indices do we need?  {0,1,2} for Q×Q, {0} otherwise.
                const std::vector<std::unique_ptr<MILPTask>> &t1 = b1->getTasks();
                const std::vector<std::unique_ptr<MILPTask>> &t2 = b2->getTasks();
                const std::vector<uint32_t> idx =
                        (t1.size() == 3 && t2.size() == 3) ? std::vector<uint32_t>{0, 1, 2} : std::vector<uint32_t>{0};
                for (const uint32_t k : idx) {
                    const MILPOperation *o1 = t1[k]->getOperations().front();
                    const MILPOperation *o2 = t2[k]->getOperations().front();
                    uint32_t dur1 = 0, dur2 = 0;
                    for (const MILPOperation *op : t1[k]->getOperations()) {
                        dur1 += op->getDuration();
                    }
                    for (const MILPOperation *op : t2[k]->getOperations()) {
                        dur2 += op->getDuration();
                    }

                    // FCFS inequality 1 (b1 before b2 when z=0):
                    // s(o2) - s(o1) - M*z >= dur1 + eps - M
                    {
                        SCIP_CONS *c;
                        std::string n = "fcfs1_" + zname + "_" + std::to_string(k);
                        // WARNING: This constraint *must* be signed, since the substraction might produce underflow
                        const int32_t lhs = dur1 + eps - bigM_;
                        LLVM_DEBUG(llvm::dbgs() << "C...lhs = " << lhs << "\n");
                        SCIPcreateConsBasicLinear(scip_, &c, n.c_str(), 0, nullptr, nullptr, lhs, SCIPinfinity(scip_));
                        SCIPaddCoefLinear(scip_, c, startVars_[o2->getId()], 1.0);
                        SCIPaddCoefLinear(scip_, c, startVars_[o1->getId()], -1.0);
                        SCIPaddCoefLinear(scip_, c, z, -static_cast<SCIP_Real>(bigM_));
                        SCIPaddCons(scip_, c);
                        SCIPreleaseCons(scip_, &c);
                    }

                    // FCFS inequality 2 (b2 before b1 when z=1):
                    // s(o1) - s(o2) + M*z >= dur2 + eps
                    {
                        SCIP_CONS *c;
                        std::string n = "fcfs2_" + zname + "_" + std::to_string(k);
                        const uint32_t lhs = dur2 + eps;
                        LLVM_DEBUG(llvm::dbgs() << "D...lhs = " << lhs << "\n");
                        SCIPcreateConsBasicLinear(scip_, &c, n.c_str(), 0, nullptr, nullptr, lhs, SCIPinfinity(scip_));
                        SCIPaddCoefLinear(scip_, c, startVars_[o1->getId()], 1.0);
                        SCIPaddCoefLinear(scip_, c, startVars_[o2->getId()], -1.0);
                        SCIPaddCoefLinear(scip_, c, z, static_cast<SCIP_Real>(bigM_));
                        SCIPaddCons(scip_, c);
                        SCIPreleaseCons(scip_, &c);
                    }
                }
                SCIPreleaseVar(scip_, &z);
            }
        }
    }

    void MILPBlockOrderModel::addIntraBlockSequencingConstraints() {
        // Enforces intra-block task precedence constraints for QL/QC blocks that follow the standard
        // 3-task structure: Task1 → Task2 → Task3.
        // For each such block:
        //  - Task2 must start *after* Task1 finishes
        //  - Task3 must start *after* Task2 finishes
        // These are encoded as inequality constraints:
        //      start(t2) >= start(t1) + dur(t1)
        //      start(t3) >= start(t2) + dur(t2)

        for (const std::shared_ptr<MILPBlock> &blk : blocks_) {
            if (blk->getType() != BlockType::QL && blk->getType() != BlockType::QC) {
                continue;
            }
            const std::vector<std::unique_ptr<MILPTask>> &tasks = blk->getTasks();
            if (tasks.size() != 3) {
                continue;
            }

            const MILPTask *t1 = tasks[0].get();
            const MILPTask *t2 = tasks[1].get();
            const MILPTask *t3 = tasks[2].get();

            const MILPOperation *first2 = t2->getOperations().front();
            const MILPOperation *first3 = t3->getOperations().front();

            int32_t dur1 = 0, dur2 = 0;
            for (const MILPOperation *op : t1->getOperations()) {
                dur1 += op->getDuration();
            }
            for (const MILPOperation *op : t2->getOperations()) {
                dur2 += op->getDuration();
            }

            // t2 after t1
            {
                SCIP_CONS *c;
                std::string n = "seq1_" + blk->getId();
                LLVM_DEBUG(llvm::dbgs() << "E...dur1 = " << dur1 << "\n");
                SCIPcreateConsBasicLinear(scip_, &c, n.c_str(), 0, nullptr, nullptr, dur1, dur1);
                SCIPaddCoefLinear(scip_, c, startVars_[first2->getId()], 1.0);
                SCIPaddCoefLinear(scip_, c, startVars_[t1->getOperations().front()->getId()], -1.0);
                SCIPaddCons(scip_, c);
                SCIPreleaseCons(scip_, &c);
            }
            // t3 after t2
            {
                SCIP_CONS *c;
                std::string n = "seq2_" + blk->getId();
                LLVM_DEBUG(llvm::dbgs() << "F...dur2 = " << dur1 << "\n");
                SCIPcreateConsBasicLinear(scip_, &c, n.c_str(), 0, nullptr, nullptr, dur2, dur2);
                SCIPaddCoefLinear(scip_, c, startVars_[first3->getId()], 1.0);
                SCIPaddCoefLinear(scip_, c, startVars_[first2->getId()], -1.0);
                SCIPaddCons(scip_, c);
                SCIPreleaseCons(scip_, &c);
            }
        }
    }

    void MILPBlockOrderModel::setPrimaryObjective() {
        // Sets the MILP objective to minimize the total qubit usage time across all qubits.
        // For each qubit:
        //  - MeasureOp start time contributes positively to the objective
        //  - AllocationOp start time contributes negatively
        // This models:
        //      Objective = sum ((start(Meas) + dur(meas)) - (start(Alloc) - dur(alloc)))
        //                = Total lifetime time per qubit
        // In this implementation we remove dur(meas) - dur(alloc) from the objective, as those are not
        // variables and have no influence on the result.

        const bool maximize = qoalaOptUnoptimize;
        SCIPsetObjsense(scip_, maximize ? SCIP_OBJSENSE_MAXIMIZE : SCIP_OBJSENSE_MINIMIZE);

        // Reset all objective coeffs to 0 first, just to be clean:
        for (auto &kv : startVars_) {
            SCIPchgVarObj(scip_, kv.second, 0.0);
        }

        SCIPsetObjsense(scip_, qoalaOptUnoptimize ? SCIP_OBJSENSE_MAXIMIZE : SCIP_OBJSENSE_MINIMIZE);

        for (const std::shared_ptr<MILPQubit> &q : qubits_) {
            const MILPOperation *alloc = q->getAllocation();
            const MILPOperation *meas = q->getMeasurement();
            // If a qubit is missing a measurement operation, we cannot meaningfully define
            // its lifetime for optimization purposes (i.e., we can't compute the interval
            // between allocation and measurement). In such cases, we exclude the qubit from
            // the objective function. Proper handling of this situation (e.g., warning or fallback)
            // is tracked under ticket #91.
            if (!(alloc && meas)) {
                continue;
            }

            SCIPchgVarObj(scip_, startVars_[alloc->getId()], -1.0);
            SCIPchgVarObj(scip_, startVars_[meas->getId()], 1.0);
        }
    }

    double MILPBlockOrderModel::getPrimaryObjectiveValueFromSolution() const {
        SCIP_SOL *bestsol = SCIPgetBestSol(scip_);
        double z = 0.0;

        for (const auto &q : qubits_) {
            const MILPOperation *alloc = q->getAllocation();
            const MILPOperation *meas = q->getMeasurement();
            if (!(alloc && meas)) {
                continue;
            }

            double sAlloc = SCIPgetSolVal(scip_, bestsol, startVars_.at(alloc->getId()));
            double sMeas = SCIPgetSolVal(scip_, bestsol, startVars_.at(meas->getId()));

            z += (sMeas - sAlloc);
        }

        return z;
    }

    void MILPBlockOrderModel::constrainPrimaryObjectiveTo(double zStar) {
        SCIP_CONS *c = nullptr;
        SCIPcreateConsBasicLinear(scip_, &c, "fix_primary_obj",
                                  /*nvars=*/0,
                                  /*vars=*/nullptr,
                                  /*vals=*/nullptr,
                                  /*lhs=*/(SCIP_Real) zStar,
                                  /*rhs=*/(SCIP_Real) zStar);

        for (const auto &q : qubits_) {
            const MILPOperation *alloc = q->getAllocation();
            const MILPOperation *meas = q->getMeasurement();
            if (!(alloc && meas)) {
                continue;
            }

            SCIPaddCoefLinear(scip_, c, startVars_.at(meas->getId()), 1.0);
            SCIPaddCoefLinear(scip_, c, startVars_.at(alloc->getId()), -1.0);
        }

        SCIPaddCons(scip_, c);
        SCIPreleaseCons(scip_, &c);
    }

    std::vector<std::string> MILPBlockOrderModel::computeAllocationOrderFromSolution() const {
        std::vector<std::pair<std::string, double>> allocWithTimes;
        SCIP_SOL *best = SCIPgetBestSol(scip_);
        if (!best) {
            llvm::errs() << "Warning: no SCIP solution available when computing allocation order.\n";
            return {};
        }

        // Collect (alloc_op_id, start_time)
        for (const auto &q : qubits_) {
            const MILPOperation *a = q->getAllocation();
            if (!a) {
                continue;
            }

            auto it = startVars_.find(a->getId());
            if (it == startVars_.end()) {
                continue;
            }

            double t = SCIPgetSolVal(scip_, best, it->second);
            allocWithTimes.emplace_back(a->getId(), t);
        }

        // Build IR order index for tie-breaking
        std::unordered_map<std::string, int> allocIR;
        int idx = 0;
        for (const auto &b : blocks_) {
            for (const auto &op : b->getOperations()) {
                allocIR[op->getId()] = idx++;
            }
        }

        // Sort by time, then by IR order
        std::sort(allocWithTimes.begin(), allocWithTimes.end(), [&](const auto &x, const auto &y) {
            if (x.second != y.second) {
                return x.second < y.second;
            }
            auto ix = allocIR.find(x.first);
            auto iy = allocIR.find(y.first);
            return (ix != allocIR.end() && iy != allocIR.end()) ? ix->second < iy->second : x.first < y.first;
        });

        // Extract IDs in order
        std::vector<std::string> allocOrder;
        allocOrder.reserve(allocWithTimes.size());
        for (const auto &p : allocWithTimes) {
            allocOrder.push_back(p.first);
        }

        return allocOrder;
    }

    void MILPBlockOrderModel::setPrimaryAllocationOrder(const std::vector<std::string> &allocOpIdsInOrder) {
        primaryAllocRank_.clear();
        int r = 0;
        for (const auto &id : allocOpIdsInOrder) {
            primaryAllocRank_[id] = r++;
        }
    }

    void MILPBlockOrderModel::setSecondaryObjectiveDeterministic() {
        // Primary fixed to z*. Choose a canonical, deterministic solution.
        //
        // Policy:
        //   - Uses the allocation order established in the primary optimization,
        //     stored in `primaryAllocRank_`.
        //   - Measurements of qubits whose allocations occurred earlier in that
        //     primary schedule receive larger weights in the secondary objective.
        //   - This ensures deterministic tie-breaking among equally optimal solutions,
        //     while preserving the exact primary objective value.
        //
        // No epsilon perturbation: this is true two-phase lexicographic optimization.
        //
        // Unoptimize mode: prefer "alloc first / meas last" by maximizing weighted lifetimes:
        //      maximize  sum_w  w * (start(meas_q) - start(alloc_q))

        // Reset objective coefficients to 0
        for (auto &kv : startVars_) {
            SCIPchgVarObj(scip_, kv.second, 0.0);
        }

        // Build (alloc op id, alloc rank, alloc op*, meas op*) records
        struct QInfo {
            const MILPOperation *alloc;
            const MILPOperation *meas;
            int rank; // smaller = earlier allocation in primary solution
        };

        std::vector<QInfo> qinfos;
        qinfos.reserve(qubits_.size());

        for (const auto &q : qubits_) {
            const MILPOperation *a = q->getAllocation();
            const MILPOperation *m = q->getMeasurement();
            if (!(a && m)) {
                continue;
            }

            auto it = primaryAllocRank_.find(a->getId());
            if (it == primaryAllocRank_.end()) {
                continue;
            }

            qinfos.push_back({a, m, it->second});
        }

        // Sort by allocation rank (earlier first)
        std::sort(qinfos.begin(), qinfos.end(), [](const QInfo &x, const QInfo &y) { return x.rank < y.rank; });

        const int K = static_cast<int>(qinfos.size());

        if (!qoalaOptUnoptimize) {
            // Optimize mode: deterministic "early measurements" tie-break
            SCIPsetObjsense(scip_, SCIP_OBJSENSE_MINIMIZE);

            for (int r = 0; r < K; ++r) {
                const MILPOperation *measOp = qinfos[r].meas;
                SCIP_VAR *v = startVars_.at(measOp->getId());
                const int w = K - r; // K, K-1, ..., 1
                SCIPchgVarObj(scip_, v, SCIPvarGetObj(v) + static_cast<SCIP_Real>(w));
            }
        } else {
            // Unoptimize mode: "alloc first / meas last" tie-break
            // Maximize weighted lifetimes (meas - alloc), with higher weight for earlier allocations.
            SCIPsetObjsense(scip_, SCIP_OBJSENSE_MAXIMIZE);

            for (int r = 0; r < K; ++r) {
                const MILPOperation *allocOp = qinfos[r].alloc;
                const MILPOperation *measOp = qinfos[r].meas;

                SCIP_VAR *vAlloc = startVars_.at(allocOp->getId());
                SCIP_VAR *vMeas = startVars_.at(measOp->getId());

                const int w = K - r; // higher weight for earlier allocs

                // +w * start(meas) - w * start(alloc)
                SCIPchgVarObj(scip_, vMeas, SCIPvarGetObj(vMeas) + static_cast<SCIP_Real>(w));
                SCIPchgVarObj(scip_, vAlloc, SCIPvarGetObj(vAlloc) - static_cast<SCIP_Real>(w));
            }
        }
    }

    double MILPModelBuilder::getOperationStartTime(const std::string &opId) const {
        auto it = startVars_.find(opId);
        if (it == startVars_.end()) {
            LLVM_DEBUG(llvm::dbgs() << "Warning: start time requested for unknown operation " << opId << "\n");
            return std::numeric_limits<double>::infinity();
        }

        SCIP_SOL *best = SCIPgetBestSol(scip_);
        if (!best) {
            LLVM_DEBUG(llvm::dbgs() << "Warning: no best solution when querying " << opId << "\n");
            return std::numeric_limits<double>::infinity();
        }

        const SCIP_Real x = SCIPgetSolVal(scip_, best, it->second);

        if (std::isnan(x) || SCIPisInfinity(scip_, x) || SCIPisInfinity(scip_, -x)) {
            LLVM_DEBUG(llvm::dbgs() << "Warning: invalid start time for " << opId << ": " << x << "\n");
            return std::numeric_limits<double>::infinity();
        }
        return static_cast<double>(x);
    }

    void MILPModelBuilder::cleanup() {
        for (auto &[id, var] : startVars_) {
            SCIPreleaseVar(scip_, &var);
        }
        startVars_.clear();

        if (scip_ != nullptr) {
            SCIPfree(&scip_);
            scip_ = nullptr;
        }
    }

    std::vector<std::string> MILPModelBuilder::getOrderedBlocks() const {
        // Returns a list of block IDs ordered by their start times as determined by the MILP solution.
        // This function computes the start and end time for each block based on the first and last operations
        // in the block, using the MILP solver's solution. The list of blocks is then sorted by their start times.

        std::vector<std::tuple<std::string, double, double>> blockTimes;

        // Collect start and end times for each block using operation timings from the MILP solution.
        for (const auto &block : blocks_) {
            const auto &ops = block->getOperations();
            if (ops.empty()) {
                continue;
            }

            const MILPOperation *firstOp = ops.front().get();
            const MILPOperation *lastOp = ops.back().get();
            double start = getOperationStartTime(firstOp->getId());
            double end = getOperationStartTime(lastOp->getId()) + static_cast<double>(lastOp->getDuration());

            blockTimes.emplace_back(block->getId(), start, end);
        }

        // Sort the blocks based on start time (ascending order).
        std::sort(blockTimes.begin(), blockTimes.end(),
                  [](const auto &a, const auto &b) { return std::get<1>(a) < std::get<1>(b); });

        for (const auto &[id, start, end] : blockTimes) {
            LLVM_DEBUG(llvm::dbgs() << "Block " << id << ": [" << start << ", " << end << "]\n");
        }

        // Extract the ordered block IDs into a list to be returned.
        std::vector<std::string> orderedBlockIds;
        orderedBlockIds.reserve(blockTimes.size());
        for (const auto &[id, start, end] : blockTimes) {
            orderedBlockIds.push_back(id);
        }

        return orderedBlockIds;
    }

    LogicalResult reorderBlocksByMilpOrder(ModuleOp &moduleOp, const std::vector<std::string> &orderedBlockIds) {
        const auto mainFuncs = moduleOp.getOps<qoalahost::MainFuncOp>();
        if (mainFuncs.empty()) {
            emitError(moduleOp.getLoc(), "No main function found in module");
            return failure();
        }
        qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();

        llvm::StringMap<Block *> idToBlock;
        Block *returnBlock = nullptr;

        // Reorder the blocks in-place in the function body based on the MILP-provided order.
        // And identify any block containing a ReturnOp.
        for (Block &blk : mainFunc) {
            for (Operation &op : blk) {
                if (llvm::isa<qoalahost::ReturnOp>(op)) {
                    returnBlock = &blk;
                    break;
                }
                if (auto meta = llvm::dyn_cast<qoalahost::BlkMeta>(op)) {
                    idToBlock[meta.getBlockId()] = &blk;
                    break;
                }
            }
        }

        // Move each block before the next insertion point; update insertion point after each move.
        auto insertionPoint = &mainFunc.front();
        llvm::DenseSet<Block *> alreadyMoved;

        // Move all QC blocks to the top (in original order)
        if (qoalaOptGroupEntReqs) {
            for (Block &blk : mainFunc) {
                bool isQC = false;
                blk.walk([&](qoalahost::BlkMeta meta) -> WalkResult {
                    if (idToBlock.contains(meta.getBlockId())) {
                        if (auto call = dyn_cast_or_null<qoalahost::CallOp>(&*std::next(blk.begin()))) {
                            isQC = dialects::helpers::hasRequestRoutineWithName(&moduleOp,
                                                                                call.getCalleeAttr().getValue());
                        }
                        if (isQC) {
                            blk.moveBefore(insertionPoint);
                            alreadyMoved.insert(&blk);
                            insertionPoint = blk.getNextNode();
                        }
                    }
                    return WalkResult::interrupt();
                });
            }
        }

        // Move MILP-ordered blocks (excluding already moved)
        for (const std::string &id : orderedBlockIds) {
            if (!idToBlock.contains(id)) {
                return moduleOp.emitError("unknown block_id \"") << id << "\"", failure();
            }

            Block *blk = idToBlock.at(id);
            if (!alreadyMoved.contains(blk)) {
                blk->moveBefore(insertionPoint);
                insertionPoint = blk->getNextNode();
            }
        }

        // Ensure returnBlock (if present) is the final block in the function body
        if (returnBlock && returnBlock->getNextNode() != nullptr) {
            returnBlock->moveBefore(nullptr);
        } // Moves to the end of the block list

        return success();
    }

    BlockPrecedenceList createPrecedenceFromOrder(ModuleOp *moduleOp, const std::vector<std::string> &orderedBlockIds,
                                                  const llvm::StringMap<MILPBlock *> &idToBlockMap) {

        BlockPrecedenceList result;

        for (size_t i = 0; i + 1 < orderedBlockIds.size(); ++i) {
            StringRef fromId = orderedBlockIds[i];
            StringRef toId = orderedBlockIds[i + 1];

            if (!idToBlockMap.contains(fromId) || !idToBlockMap.contains(toId)) {
                moduleOp->emitWarning() << "Warning: Missing block in map for precedence: " << fromId << " or " << toId;
                continue;
            }

            result.emplace_back(idToBlockMap.at(fromId), idToBlockMap.at(toId));
        }

        LLVM_DEBUG({
            llvm::dbgs() << "[MILP] Deadlines precedence edges:\n";
            for (const auto &[precedenceA, precedenceB] : result) {
                llvm::dbgs() << "  " << precedenceA->getId() << " -> " << precedenceB->getId() << "\n";
            }
        });

        return result;
    }

    void MILPBlockDeadlineModel::createVariables() {
        // Variables created here:
        //   - s_op (start time for each operation)
        //   - g_min (scalar >= 0): the minimum inter-block gap to maximize
        //   - G_pred_to_succ (>= 0 for each consecutive pair in known order)
        // Assumption: `precedences_` define a single chain. We do not reconstruct
        // a separate order; we directly iterate over `precedences_` to create the gap vars.

        // Create start time variables for all operations
        for (const auto &blk : blocks_) {
            for (const auto &op : blk->getOperations()) {
                const std::string &id = op->getId();
                if (!startVars_.count(id)) {
                    startVars_[id] = createVariable(scip_, "s_" + id, /*strictlyPositive=*/false);
                }
            }
        }

        // Zero all objective coefficients on start vars (clean slate)
        for (auto &kv : startVars_) {
            SCIPchgVarObj(scip_, kv.second, 0.0);
        }

        // g_min
        gminVar_ = createVariable(scip_, "g_min", /*strictlyPositive=*/false);

        // Create gap variables G for each precedence edge (pred -> succ)
        gapVars_.clear();
        for (const auto &[pred, succ] : precedences_) {
            const std::string gname = "G_" + pred->getId() + "_to_" + succ->getId();
            gapVars_[gname] = createVariable(scip_, gname, /*strictlyPositive=*/false);
        }
    }

    void MILPBlockDeadlineModel::setPrimaryObjective() {
        // Sets the objective: maximize the minimum inter-block gap g_min.

        SCIPsetObjsense(scip_, SCIP_OBJSENSE_MAXIMIZE);
        for (auto &kv : startVars_) {
            SCIPchgVarObj(scip_, kv.second, 0.0);
        }
        for (auto &kv : gapVars_) {
            SCIPchgVarObj(scip_, kv.second, 0.0);
        }
        if (gminVar_) {
            SCIPchgVarObj(scip_, gminVar_, 1.0);
        }
    }

    void MILPBlockDeadlineModel::addIntraTaskSequencingConstraints() {
        // Enforces strict sequential execution of operations within each task.
        // For each task and each consecutive operation pair (o1, o2):
        //    - start(o2) = start(o1) + dur(o1)
        // This is encoded as a linear equality constraint:
        //    - start(o2) - start(o1) = dur(o1)

        for (const auto &blk : blocks_) {
            for (const auto &t : blk->getTasks()) {
                const auto &ops = t->getOperations();
                for (size_t i = 0; i + 1 < ops.size(); ++i) {
                    const auto *o1 = ops[i];
                    const auto *o2 = ops[i + 1];

                    SCIP_CONS *c;
                    std::string name = "intra_task_" + o1->getId() + "_" + o2->getId();
                    const int32_t rhs = o1->getDuration();
                    LLVM_DEBUG(llvm::dbgs() << "G...rhs = " << rhs << "\n");
                    SCIPcreateConsBasicLinear(scip_, &c, name.c_str(), 0, nullptr, nullptr, rhs, rhs);
                    SCIPaddCoefLinear(scip_, c, startVars_[o2->getId()], 1.0);
                    SCIPaddCoefLinear(scip_, c, startVars_[o1->getId()], -1.0);
                    SCIPaddCons(scip_, c);
                    SCIPreleaseCons(scip_, &c);
                }
            }
        }
    }

    void MILPBlockDeadlineModel::addIntraBlockSequencingConstraints() {
        // Enforces strict execution order of tasks within each block.
        // For each consecutive task pair (t1, t2) in a block:
        //    - start(first_op(t2)) = start(last_op(t1)) + dur(last_op(t1))
        // Ensures task2 starts exactly after task1 ends.
        // Encoded as equality: start2 - start1 = dur

        for (const auto &blk : blocks_) {
            const auto &tasks = blk->getTasks();
            for (size_t i = 0; i + 1 < tasks.size(); ++i) {
                const auto *t1 = tasks[i].get();
                const auto *t2 = tasks[i + 1].get();
                const MILPOperation *last1 = t1->getOperations().back();
                const MILPOperation *first2 = t2->getOperations().front();
                const int32_t dur1 = last1->getDuration();

                std::string name = "intra_block_" + blk->getId() + "_" + std::to_string(i);
                SCIP_CONS *c;

                // Enforce strict equality: LHS == RHS == dur1
                LLVM_DEBUG(llvm::dbgs() << "I...dur1 = " << dur1 << "\n");
                SCIPcreateConsBasicLinear(scip_, &c, name.c_str(), 0, nullptr, nullptr, dur1, dur1);
                SCIPaddCoefLinear(scip_, c, startVars_[first2->getId()], 1.0);
                SCIPaddCoefLinear(scip_, c, startVars_[last1->getId()], -1.0);
                SCIPaddCons(scip_, c);
                SCIPreleaseCons(scip_, &c);
            }
        }
    }

    void MILPBlockDeadlineModel::addBlockPrecedenceConstraints() {
        // Enforces global block-level precedence constraints.
        // For each edge (pred -> succ) in the dependency graph:
        //    - start(first_op(succ)) => start(last_op(pred)) + dur(last_op(pred))
        // This guarantees causal order between blocks.
        // Encoded as a linear inequality with a lower bound.

        for (const auto &[pred, succ] : precedences_) {
            const MILPOperation *lastPred = pred->lastOp();
            const MILPOperation *firstSucc = succ->firstOp();

            const uint32_t dur = lastPred->getDuration();

            SCIP_CONS *c;
            std::string name = "block_prec_" + pred->getId() + "_" + succ->getId();
            LLVM_DEBUG(llvm::dbgs() << "J...dur = " << dur << "\n");
            SCIPcreateConsBasicLinear(scip_, &c, name.c_str(), 0, nullptr, nullptr, dur, SCIPinfinity(scip_));
            SCIPaddCoefLinear(scip_, c, startVars_[firstSucc->getId()], 1.0);
            SCIPaddCoefLinear(scip_, c, startVars_[lastPred->getId()], -1.0);
            SCIPaddCons(scip_, c);
            SCIPreleaseCons(scip_, &c);
        }
    }

    void MILPBlockDeadlineModel::addFCFSConsistencyConstraints() {
        // Adds First-Come-First-Served (FCFS) consistency constraints for blocks with known order.
        // For each precedence pair (b1 -> b2), and for each task index k:
        //    - Ensure: start(o2) => start(o1) + total_dur(t1[k])
        //      where o1/o2 are first operations of tasks in position k of b1 and b2.
        // Applied when both blocks are of matching quantum structure (3 tasks) or single-task classical blocks.
        // This helps preserve expected temporal consistency when reordering blocks

        for (const auto &[b1, b2] : precedences_) {
            const auto &t1 = b1->getTasks();
            const auto &t2 = b2->getTasks();

            std::vector<uint32_t> indices =
                    (t1.size() == 3 && t2.size() == 3) ? std::vector<uint32_t>{0, 1, 2} : std::vector<uint32_t>{0};

            for (const uint32_t k : indices) {
                if (k >= t1.size() || k >= t2.size()) {
                    llvm::errs() << "Warning: FCFS index out of range in blocks " << b1->getId() << " or "
                                 << b2->getId() << " at k=" << k << "\n";
                    continue;
                }

                const MILPOperation *o1 = t1[k]->getOperations().front();
                const MILPOperation *o2 = t2[k]->getOperations().front();

                uint32_t dur = 0;
                for (const MILPOperation *op : t1[k]->getOperations()) {
                    dur += op->getDuration();
                }

                std::string name = "fcfs_" + b1->getId() + "_" + b2->getId() + "_" + std::to_string(k);
                SCIP_CONS *c;
                LLVM_DEBUG(llvm::dbgs() << "K...dur = " << dur << "\n");
                SCIPcreateConsBasicLinear(scip_, &c, name.c_str(), 0, nullptr, nullptr, dur, SCIPinfinity(scip_));
                SCIPaddCoefLinear(scip_, c, startVars_[o2->getId()], 1.0);
                SCIPaddCoefLinear(scip_, c, startVars_[o1->getId()], -1.0);
                SCIPaddCons(scip_, c);
                SCIPreleaseCons(scip_, &c);
            }
        }
    }

    void MILPBlockDeadlineModel::addInterBlockGapConstraints() {
        // Enforces gap definitions and min-gap floor for each precedence edge:
        //   - Define gap:
        //       G_pred_to_succ = start(first(succ)) - start(last(pred)) - dur(last(pred))
        //   - Enforce minimum gap:
        //       G_pred_to_succ >= g_min
        // Gap variables have LB >= 0 by construction.

        for (const auto &[pred, succ] : precedences_) {

            const MILPOperation *lastPred = pred->lastOp();
            const MILPOperation *firstSucc = succ->firstOp();
            if (!(lastPred && firstSucc)) {
                continue;
            }

            // WARNING - we need to make thi variable signed, since we are using it as a negative
            // when passing it to the connstraint creator
            const int32_t durLastPred = lastPred->getDuration();
            const std::string gname = "G_" + pred->getId() + "_to_" + succ->getId();
            SCIP_VAR *G = gapVars_.at(gname);

            // Gap definition:  G - s(firstSucc) + s(lastPred) = -durLastPred
            {
                SCIP_CONS *c;
                std::string cname = "gap_def_" + pred->getId() + "_" + succ->getId();
                SCIPcreateConsBasicLinear(scip_, &c, cname.c_str(), 0, nullptr, nullptr, -durLastPred, -durLastPred);
                SCIPaddCoefLinear(scip_, c, G, 1.0);
                SCIPaddCoefLinear(scip_, c, startVars_.at(firstSucc->getId()), -1.0);
                SCIPaddCoefLinear(scip_, c, startVars_.at(lastPred->getId()), 1.0);
                SCIPaddCons(scip_, c);
                SCIPreleaseCons(scip_, &c);
            }

            // Minimum gap:  G - g_min >= 0
            {
                SCIP_CONS *c;
                std::string cname = "gap_ge_gmin_" + pred->getId() + "_" + succ->getId();
                SCIPcreateConsBasicLinear(scip_, &c, cname.c_str(), 0, nullptr, nullptr, 0.0, SCIPinfinity(scip_));
                SCIPaddCoefLinear(scip_, c, G, 1.0);
                SCIPaddCoefLinear(scip_, c, gminVar_, -1.0);
                SCIPaddCons(scip_, c);
                SCIPreleaseCons(scip_, &c);
            }
        }
    }

    void MILPBlockDeadlineModel::addProgramHorizonConstraint() {
        // Constrains the program to finish before a global time horizon M.
        // We enforce: end(tail block) <= M
        // -> s(lastOp(tail)) + dur(lastOp) <= M.
        // The tail is the unique block that never appears as a predecessor in `precedences_`.
        // Assumptions:
        // - The precedences define a chain, thus the tail exists,
        // - The MILPBlocks are well formed, thus there is at least one (non blk_meta) op per block.

        // Find tail: appears only as succ, never as pred
        llvm::DenseSet<const MILPBlock *> preds, succs;
        for (const auto &[first, second] : precedences_) {
            preds.insert(first);
            succs.insert(second);
        }
        const MILPBlock *tail = nullptr;
        for (const MILPBlock *cand : succs) {
            if (!preds.contains(cand)) {
                tail = cand;
                break;
            }
        }

        assert(tail && "[Deadlines][Horizon] Invariant violated: no tail block found from precedences");

        const MILPOperation *lastOp = tail->lastOp();
        const double H = getProgramHorizon();
        const double ubOnStart = H - static_cast<double>(lastOp->getDuration());

        SCIP_CONS *c;
        std::string name = "program_horizon";
        // s(lastOp) <= M - dur(lastOp)
        SCIPcreateConsBasicLinear(scip_, &c, name.c_str(), 0, nullptr, nullptr, -SCIPinfinity(scip_), ubOnStart);
        SCIPaddCoefLinear(scip_, c, startVars_.at(lastOp->getId()), 1.0);
        SCIPaddCons(scip_, c);
        SCIPreleaseCons(scip_, &c);
    }

    void MILPBlockDeadlineModel::addQubitLifetimeConstraints() {
        // Constrains each qubit's lifetime to stay within Lmax (feasibility only).
        // For each qubit q with alloc and meas:
        //   (s_meas + dur_meas) - (s_alloc + dur_alloc) <= Lmax
        // -> s_meas - s_alloc <= Lmax - dur_meas + dur_alloc

        for (const auto &q : qubits_) {
            const std::string &id = q->getId();
            const MILPOperation *alloc = q->getAllocation();
            const MILPOperation *meas = q->getMeasurement();

            if (!meas) {
                LLVM_DEBUG(llvm::dbgs() << "Skipping qubit " << id << " — missing meas operation.\n");
                continue;
            }

            const uint32_t allocDur = alloc->getDuration();
            const uint32_t measDur = meas->getDuration();
            const uint32_t Lmax = qoalaOptQubitLifetime;

            const int32_t rhs = Lmax - measDur + allocDur;

            SCIP_CONS *c;
            std::string name = "lifetime_" + id;
            LLVM_DEBUG(llvm::dbgs() << "L...rhs = " << rhs << "\n");
            SCIPcreateConsBasicLinear(scip_, &c, name.c_str(), 0, nullptr, nullptr, -SCIPinfinity(scip_), rhs);
            SCIPaddCoefLinear(scip_, c, startVars_.at(meas->getId()), 1.0);
            SCIPaddCoefLinear(scip_, c, startVars_.at(alloc->getId()), -1.0);
            SCIPaddCons(scip_, c);
            SCIPreleaseCons(scip_, &c);
        }
    }

    double MILPBlockDeadlineModel::getProgramHorizon() const {
        // Model: Determine a conservative global time bound M for the program.
        // Default: M_default = 2 * sum_{ops} duration(op).
        // If a positive qoalaOptProgramHorizon is provided:
        //   - If qoalaOptProgramHorizon < sumDur, emit a warning and use the default (too tight).
        //   - Else, accept and use qoalaOptProgramHorizon.
        // Otherwise, use the default.

        // Sum of all operation durations
        int64_t sumDur = 0;
        for (const auto &blk : blocks_) {
            for (const auto &op : blk->getOperations()) {
                sumDur += op->getDuration();
            }
        }

        const double sumDurD = static_cast<double>(sumDur);
        const double defaultH = 2.0 * sumDurD; // conservative default
        const double userHorizon = static_cast<double>(qoalaOptProgramHorizon);

        // If user provided a positive horizon, validate and use it (or fall back with warning)
        if (qoalaOptProgramHorizon > 0) {
            if (userHorizon < sumDurD) {
                // emit warning that the inputted horizon is too low and that we are going to default
                llvm::errs() << "[Deadlines] Provided program horizon (" << userHorizon
                             << ") is smaller than the aggregate duration lower bound (" << sumDurD
                             << "). Falling back to default horizon (" << defaultH << ").\n";
                return defaultH;
            }
            // use qoalaOptProgramHorizon
            LLVM_DEBUG(llvm::dbgs() << "[Deadlines] Using user-provided program horizon: " << userHorizon << "\n");
            return userHorizon;
        }

        // default
        LLVM_DEBUG(llvm::dbgs() << "[Deadline] Using default program horizon (2 * sumDur): " << defaultH << "\n");
        return defaultH;
    }

    double MILPBlockDeadlineModel::getPrimaryObjectiveValueFromSolution() const {
        SCIP_SOL *best = SCIPgetBestSol(scip_);
        return SCIPgetSolVal(scip_, best, gminVar_);
    }

    void MILPBlockDeadlineModel::constrainPrimaryObjectiveTo(double zStar) {
        // Constrain gmin == zStar (exact lexicographic lock)
        SCIP_CONS *c = nullptr;
        SCIPcreateConsBasicLinear(scip_, &c, "fix_gmin",
                                  /*nvars=*/0, /*vars=*/nullptr, /*vals=*/nullptr,
                                  /*lhs=*/(SCIP_Real) zStar,
                                  /*rhs=*/(SCIP_Real) zStar);

        // gmin has coefficient +1
        SCIPaddCoefLinear(scip_, c, gminVar_, 1.0);

        SCIPaddCons(scip_, c);
        SCIPreleaseCons(scip_, &c);
    }

    void MILPBlockDeadlineModel::setSecondaryObjectiveDeterministic() {
        // Sets the deterministic secondary objective of the
        // lexicographic (two-phase) optimization for the deadline model.
        // Policy:
        //   - The secondary objective is not physically meaningful; it is used
        //     solely to enforce deterministic, platform-independent ordering.
        //   - We assign small, monotonically decreasing integer weights to the
        //     start-time variables of operations according to their order in the
        //     compiler’s intermediate representation (IR).
        //   - Minimizing this weighted sum promotes an ALAP
        //     bias within the feasible region defined by g_min = z*, without
        //     changing any primary optimal values.
        //   - No epsilon perturbation is used: the primary objective is *exactly*
        //     fixed via constraint, and this objective only acts as a deterministic
        //     selector among degenerate optima.

        SCIPsetObjsense(scip_, SCIP_OBJSENSE_MINIMIZE);

        // Zero all objective coefficients
        SCIPchgVarObj(scip_, gminVar_, 0.0);
        for (auto &kv : startVars_) {
            SCIPchgVarObj(scip_, kv.second, 0.0);
        }
        for (auto &kv : gapVars_) {
            SCIPchgVarObj(scip_, kv.second, 0.0);
        }

        // Build a stable global IR order over all operations
        std::vector<const MILPOperation *> opsInIR;
        opsInIR.reserve([&] {
            size_t n = 0;
            for (const auto &b : blocks_) {
                n += b->getOperations().size();
            }
            return n;
        }());

        for (const auto &b : blocks_) {
            for (const auto &op : b->getOperations()) {
                opsInIR.push_back(op.get());
            }
        }

        // Assign strictly increasing integer weights by IR order:
        //   earlier IR op -> smaller weight, later IR op -> larger weight.
        const int N = static_cast<int>(opsInIR.size());
        for (int i = 0; i < N; ++i) {
            const MILPOperation *op = opsInIR[i];
            auto it = startVars_.find(op->getId());
            if (it == startVars_.end()) {
                continue; // safety
            }
            SCIP_VAR *v = it->second;
            const int w = i + 1;
            SCIPchgVarObj(scip_, v, SCIPvarGetObj(v) + static_cast<SCIP_Real>(w));
        }
    }

    std::pair<std::unordered_map<std::string, uint32_t>, std::string>
    MILPBlockDeadlineModel::computeBlockDeadlines() const {
        // Computes relative deadlines for each block based on their actual scheduled start times.
        // The deadlines represent how many time units after the end of the reference block each block should begin.
        // Steps:
        //   1. Retrieve the topologically ordered list of blocks.
        //   2. Choose the *reference block* (first in the list), and calculate its end time.
        //   3. For every other block:
        //       - Calculate its start time relative to the reference block's end.
        //       - If that would result in a negative deadline (i.e., block starts before the reference finishes),
        //         assign a deadline just after the last valid one (to avoid non-monotonicity).
        //   4. Return the mapping: block ID -> deadline offset, and the ID of the reference block.
        // These deadlines are later used to annotate MLIR `BlkMeta` operations with scheduling guidance.

        std::unordered_map<std::string, uint32_t> deadlines;

        const std::vector<std::string> ordered = getOrderedBlocks();
        if (ordered.empty()) {
            return {deadlines, ""};
        }

        // Reference block is the first in the ordered list
        const std::string &refBlockId = ordered.front();
        const MILPBlock *refBlock = nullptr;

        for (const auto &blk : blocks_) {
            if (blk->getId() == refBlockId) {
                refBlock = blk.get();
                break;
            }
        }

        if (!refBlock || refBlock->getOperations().empty()) {
            return {deadlines, ""};
        }

        const auto &refOps = refBlock->getOperations();
        const MILPOperation *lastRefOp = refOps.back().get();
        const double refEnd = getOperationStartTime(lastRefOp->getId()) + static_cast<double>(lastRefOp->getDuration());

        uint32_t lastValidDeadline = 0; // Initial baseline

        for (auto &blkId : ordered) {
            const MILPBlock *blk = nullptr;
            for (const auto &b : blocks_) {
                if (b->getId() == blkId) {
                    blk = b.get();
                    break;
                }
            }

            if (!blk || blk->getOperations().empty()) {
                continue;
            }

            const auto *startOp = blk->getOperations().front().get();
            const double startTime = getOperationStartTime(startOp->getId());
            // WARNING - This next difference can be negative, so we need to use a signed integer
            int32_t deadline = static_cast<int32_t>(std::round(startTime - refEnd));

            // Correct negative deadline by using last valid + 1
            if (deadline < 0) {
                deadline = lastValidDeadline + 1;
            }

            deadlines[blkId] = deadline;
            lastValidDeadline = deadline;
        }

        LLVM_DEBUG({
            llvm::dbgs() << "Reference block id: " << refBlockId << "\n";
            for (const auto &[blkId, deadline] : deadlines) {
                llvm::dbgs() << "[DeadlineModel] Deadline for block " << blkId << ": " << deadline << "\n";
            }
        });

        return {deadlines, refBlockId};
    }

    void annotateBlockDeadlines(ModuleOp &moduleOp, const std::unordered_map<std::string, uint32_t> &deadlines,
                                const std::string &refBlockId) {
        // Annotates MLIR `BlkMeta` operations with computed block deadlines.
        // Parameters:
        //   - moduleOp: the MLIR module containing the program
        //   - deadlines: mapping from block ID to its relative deadline
        //   - refBlockId: the reference block, which has no deadline
        // For each `BlkMeta` in the main function:
        //   - If it matches a known block ID and isn't the reference block:
        //       - Add a new dictionary attribute: { refBlockId: deadline }
        //         This indicates the number of time units after the reference block's completion
        //         that this block is expected to begin.

        LLVM_DEBUG(llvm::dbgs() << "[Deadlines] Annoatating BlkMeta operations...\n");

        const auto mainFuncs = moduleOp.getOps<qoalahost::MainFuncOp>();
        if (mainFuncs.empty()) {
            emitError(moduleOp.getLoc(), "No main function found in module");
            return;
        }
        qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();
        mainFunc.walk([&](qoalahost::BlkMeta blkMeta) {
            const std::string blkId = blkMeta.getBlockId().str();

            if (blkId == refBlockId) {
                return; // No deadline for the reference block
            }

            const auto it = deadlines.find(blkId);
            if (it == deadlines.end()) {
                LLVM_DEBUG(llvm::dbgs() << "[DeadlineModel] No deadline found for block " << blkId << "\n");
                return;
            }

            const uint32_t deadline = it->second;

            LLVM_DEBUG(llvm::dbgs() << "[Deadlines] Block ID: " << blkId << ", deadline: " << deadline << "\n");

            const auto ctx = blkMeta->getContext();
            const auto deadlineAttr = IntegerAttr::get(IntegerType::get(ctx, 64), deadline);
            const auto dictAttr =
                    DictionaryAttr::get(ctx, {NamedAttribute(StringAttr::get(ctx, refBlockId), deadlineAttr)});
            blkMeta.setDeadlinesAttr(dictAttr);
        });
    }
} // namespace qoala::analysis::reordering
