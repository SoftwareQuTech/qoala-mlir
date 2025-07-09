#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "Dialect/QoalaHost/Passes.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Tools/QoalaOpt.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormatVariadic.h"
#include "mlir/IR/SymbolTable.h"

#include "scip/scip.h"
#include "scip/scipdefplugins.h"

#define DEBUG_TYPE "qoalahost-reorder-blocks-pass-internal"

using namespace mlir;
using namespace qoala::dialects;
using namespace qoala::analysis;

namespace qoala::analysis::reordering {
    static int getOperationDuration(Operation *op) {
        // Returns the execution duration (in nanoseconds) of a given operation
        // to be used in the MILP model.

        // CallOp and NopOp are the Pre and Post tasks
        if (isa<qoalahost::CallOp>(op) || isa<qoalahost::NopOp>(op)) {
            return qoalaOptHostInstrTime;
        }

        // TODO - check in translation if the size of tensor decides the number of send ops.
        // After tensor lowering in #72.
        if (isa<qoalahost::SendIntsOp>(op) || isa<qoalahost::SendFloatsOp>(op)) {
            return qoalaOptHostInstrTime;
        }

        // TODO - check in translation if the size of tensor decides the number of recv ops.
        // After tensor lowering in #72.
        if (isa<qoalahost::RecvIntsOp>(op) || isa<qoalahost::RecvFloatsOp>(op)) {
            return qoalaOptLatency + qoalaOptHostPeerLatency;
        };

        // Netqasm operations that are classical operations
        if (isa<netqasm::QAllocOp>(op) || isa<netqasm::QFreeOp>(op)) {
            return qoalaOptQNosInstrTime;
        };

        // Each operand will be converted to one netqasm instruction in the iqoala file
        if (isa<netqasm::ReturnOp>(op)) {
            return op->getNumOperands() * qoalaOptQNosInstrTime;
        }

        // Single qubit gate operations, qubit init and measure
        if (isa<netqasm::QInitOp>(op) || isa<netqasm::RotateXOp>(op) || isa<netqasm::RotateYOp>(op) ||
            isa<netqasm::RotateZOp>(op) || isa<netqasm::HadamardOp>(op) || isa<netqasm::MeasureOp>(op)) {
            return qoalaOptSingleGateDuration;
        }

        // Two qubits gate oeprations
        if (isa<netqasm::CnotOp>(op) || isa<netqasm::CzOp>(op) || isa<netqasm::CrotXOp>(op)) {
            return qoalaOptTwoGateDuration;
        }

        if (isa<netqasm::EprsOp>(op)) {
            return qoalaOptLinkDuration;
        }

        if (isa<netqasm::EprsMeasureOp>(op)) {
            return qoalaOptLinkDuration + qoalaOptSingleGateDuration;
        }

        // Any other instructions are the ones in CL blocks
        return qoalaOptHostInstrTime;
    }

    static Operation *resolveCallee(qoalahost::CallOp callOp, const llvm::StringMap<Operation *> &routineMap) {
        // Fast lookup of the callee routine for a qoalahost.call.
        // Returns nullptr if the symbol is not in `routineMap`.
        // `getCallee()` is cheaper than going through SymbolRefAttr plumbing.
        StringRef symName = callOp.getCallee();
        auto it = routineMap.find(symName);
        return (it != routineMap.end()) ? it->second : nullptr;
    }

    static OpType getBlockType(Operation *op, llvm::StringMap<Operation *> routineMap) {
        // Infers the block type based on the given operation (typically the first
        // non BlkMeta operation in a block).

        // First, handle communication ops (CC)
        if (llvm::isa<qoalahost::SendIntsOp>(op) || llvm::isa<qoalahost::RecvIntsOp>(op) ||
            llvm::isa<qoalahost::SendFloatsOp>(op) || llvm::isa<qoalahost::RecvFloatsOp>(op)) {
            return OpType::CC;
        }

        // Then check for call-based classification
        if (auto callOp = llvm::dyn_cast<qoalahost::CallOp>(op)) {
            Operation *callee = resolveCallee(callOp, routineMap);
            if (!callee) {
                return OpType::CL;
            }

            if (isa<netqasm::RequestRoutineOp>(callee)) {
                return OpType::QC;
            }
            if (isa<netqasm::LocalRoutineOp>(callee)) {
                return OpType::QL;
            }
        }

        // Default fallback
        return OpType::CL;
    }

    static LogicalResult createTasksForBlock(MILPBlock *blk, const Location &loc) {
        // Creates the set of MILP tasks associated with a given block.

        const std::vector<std::unique_ptr<MILPOperation>> &ops = blk->getOperations();
        if (ops.empty()) {
            emitError(loc) << "MILPBlock '" << blk->getId() << "' has no operations — this should not happen.";
            return failure();
        }

        switch (blk->getType()) {
            case OpType::CC:
                if (ops.size() != 1) {
                    emitError(loc) << "MILPBlock of type CC should contain exactly one operation, but got "
                                   << ops.size();
                    return failure();
                }
            case OpType::CL: {
                std::unique_ptr<MILPTask> task = std::make_unique<MILPTask>("0", blk, TaskGroup::C);
                for (const std::unique_ptr<MILPOperation> &op : ops) {
                    task->addOperation(op.get());
                }

                blk->addTask(std::move(task));
                break;
            }
            case OpType::QL:
            case OpType::QC: {
                if (ops.size() < 3) {
                    emitError(loc) << "QL/QC block must contain at least 3 operations to form 3 tasks";
                    return failure();
                }
                // Task 0 – call (C): PreTask
                std::unique_ptr<MILPTask> t0 = std::make_unique<MILPTask>("0", blk, TaskGroup::C);
                t0->addOperation(ops.front().get());
                blk->addTask(std::move(t0));
                // Task 1 – middle (Q): Quantum Routine
                std::unique_ptr<MILPTask> t1 = std::make_unique<MILPTask>("1", blk, TaskGroup::Q);
                for (size_t i = 1; i + 1 < ops.size(); ++i) {
                    t1->addOperation(ops[i].get());
                }
                blk->addTask(std::move(t1));
                // Task 2 – return (C) : PostTask.
                std::unique_ptr<MILPTask> t2 = std::make_unique<MILPTask>("2", blk, TaskGroup::C);
                t2->addOperation(ops.back().get());
                blk->addTask(std::move(t2));
                break;
            }
        }
        return success();
    }

    static LogicalResult inlineCallIntoBlock(qoalahost::CallOp callOp, const std::string &blkId, OpType blkType,
                                             int &opIdx, const llvm::StringMap<Operation *> &routineMap,
                                             mlir::Block *callerBlock, MILPBlock *blk,
                                             std::unordered_map<mlir::Operation *, MILPOperation *> &opToMilpOp) {
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
        auto calleeFunc = llvm::cast<FunctionOpInterface>(callee);

        bool foundReturn = false;

        // Inline every op from the callee
        calleeFunc->getRegion(0).walk([&](Operation *innerOp) -> WalkResult {
            // Create MILPOperation for every inlined op
            std::string subId = blkId + "::" + std::to_string(opIdx++);
            std::unique_ptr<MILPOperation> milpSub =
                    std::make_unique<MILPOperation>(subId, blkType, getOperationDuration(innerOp));
            milpSub->setOperation(innerOp);
            MILPOperation *raw = blk->addOperation(std::move(milpSub));
            opToMilpOp[innerOp] = raw;

            // When we hit the netqasm.return, insert the synthetic nop
            if (llvm::isa<netqasm::ReturnOp>(innerOp)) {
                OpBuilder builder(callOp.getContext());
                builder.setInsertionPointToEnd(callerBlock);
                qoalahost::NopOp nop = builder.create<qoalahost::NopOp>(innerOp->getLoc());

                std::string nopId = blkId + "::" + std::to_string(opIdx++);
                std::unique_ptr<MILPOperation> milpNop =
                        std::make_unique<MILPOperation>(nopId, blkType, getOperationDuration(nop.getOperation()));
                milpNop->setOperation(nop.getOperation());
                MILPOperation *rawNop = blk->addOperation(std::move(milpNop));
                opToMilpOp[nop.getOperation()] = rawNop;

                foundReturn = true;
                return WalkResult::interrupt(); // stop the walk
            }
            return WalkResult::advance();
        });

        if (!foundReturn) {
            return callOp.emitError("Callee does not end in netqasm::ReturnOp"), failure();
        }

        return success();
    }

    static void recordEdge(StringRef predId, const llvm::StringMap<MILPBlock *> &idToBlockMap,
                           std::vector<std::pair<MILPBlock *, MILPBlock *>> &precedences,
                           std::vector<std::pair<std::string, std::string>> &unresolvedEdges, MILPBlock *blk,
                           const std::string &blkId) {
        if (predId.empty()) {
            return;
        }

        auto it = idToBlockMap.find(predId.str());
        if (it != idToBlockMap.end()) {
            precedences.emplace_back(it->second, blk);
        } else {
            unresolvedEdges.emplace_back(predId.str(), blkId);
        }
    }

    static llvm::StringMap<Operation *> collectRoutineMap(ModuleOp moduleOp) {
        llvm::StringMap<Operation *> routineMap;

        moduleOp.walk([&](helpers::NetQASMRoutineInterface routine) {
            Operation *op = routine.getOperation();

            if (auto localRoutine = llvm::dyn_cast<netqasm::LocalRoutineOp>(op)) {
                routineMap.try_emplace(localRoutine.getSymName(), op);
            } else if (auto requestRoutine = llvm::dyn_cast<netqasm::RequestRoutineOp>(op)) {
                routineMap.try_emplace(requestRoutine.getSymName(), op);
            }
        });

        return routineMap;
    }

    static LogicalResult buildMilpBlocks(qoalahost::MainFuncOp mainFunc, const llvm::StringMap<Operation *> &routineMap,
                                         std::vector<std::shared_ptr<MILPBlock>> &outBlocks,
                                         BlockPrecedenceList &outPrecedences,
                                         llvm::StringMap<MILPBlock *> &idToBlockMap,
                                         std::unordered_map<mlir::Operation *, MILPOperation *> &opToMilpOp,
                                         std::vector<std::pair<std::string, std::string>> &unresolvedEdges) {
        llvm::DenseSet<Block *> visitedBlocks;
        LogicalResult status = success();

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

            Block::iterator firstIt = std::next(block->begin());
            Operation *firstOp = &*firstIt;
            OpType blkType = getBlockType(firstOp, routineMap);

            // Create new MILPBlock object and associate with block ID and type
            std::string blkId = blkMeta.getBlockId().str();
            std::shared_ptr<MILPBlock> blkPtr = std::make_shared<MILPBlock>(blkId, blkType);
            blkPtr->setBlock(block);
            MILPBlock *blk = blkPtr.get();
            idToBlockMap[blkId] = blk;

            // Index operations to create unique IDs for MILPOperation
            int opIdx = 0;

            // Walk through the actual instructions in the MLIR block
            bool isFirstOp = true;
            for (Operation &op : *block) {
                if (isFirstOp) {
                    isFirstOp = false;
                    continue; // Skip BlkMeta
                }

                // Stop early if we reach a return/nop_term in a classical block
                if (blkType == OpType::CL && llvm::isa<qoalahost::NopTOp>(op)) {
                    break;
                }

                // Create MILPOperation
                std::string opId = blkId + "::" + std::to_string(opIdx++);
                std::unique_ptr<MILPOperation> milpOp =
                        std::make_unique<MILPOperation>(opId, blkType, getOperationDuration(&op));
                milpOp->setOperation(&op);
                MILPOperation *raw = blk->addOperation(std::move(milpOp));
                opToMilpOp[&op] = raw;

                // Handle inlining for quantum-local or quantum-communication blocks
                // - Inline body of the called routine
                // - Track operations
                // - Add a qoalahost.nop at the end to model post-task transition
                if (blkType == OpType::QL || blkType == OpType::QC) {
                    if (auto callOp = llvm::dyn_cast<qoalahost::CallOp>(op)) {
                        if (failed(inlineCallIntoBlock(callOp, blkId, blkType, opIdx, routineMap,
                                                       block, // callerBlock
                                                       blk, opToMilpOp))) {
                            status = failure();
                            return WalkResult::interrupt();
                        }
                        break; // only one call per block
                    }
                }

                // CC blocks contain a single communication operation only
                if (blkType == OpType::CC) {
                    break;
                }
            }

            if (failed(status)) {
                return WalkResult::interrupt();
            }

            // Record precedence edges from block attributes. For our optimization all types of precedences are
            // equivalent. The differenciation is needed for the translation step.
            if (ArrayAttr a = blkMeta.getPredecessorsAttr()) {
                for (llvm::StringRef s : a.getAsValueRange<StringAttr>()) {
                    recordEdge(s, idToBlockMap, outPrecedences, unresolvedEdges, blk, blkId);
                }
            }
            if (ArrayAttr a = blkMeta.getDependenciesAttr()) {
                for (llvm::StringRef s : a.getAsValueRange<StringAttr>()) {
                    recordEdge(s, idToBlockMap, outPrecedences, unresolvedEdges, blk, blkId);
                }
            }
            if (StringAttr a = blkMeta.getPrevEntAttr()) {
                recordEdge(a.getValue(), idToBlockMap, outPrecedences, unresolvedEdges, blk, blkId);
            }
            if (StringAttr a = blkMeta.getPrevCommAttr()) {
                recordEdge(a.getValue(), idToBlockMap, outPrecedences, unresolvedEdges, blk, blkId);
            }

            // Generate tasks for this MILP block (task-level subdivision)
            if (failed(createTasksForBlock(blk, blkMeta.getLoc()))) {
                status = failure();
                return WalkResult::interrupt();
            }

            outBlocks.push_back(std::move(blkPtr));
            return WalkResult::advance();
        });

        return status;
    }

    static LogicalResult collectQubitUsage(qoalahost::MainFuncOp mainFunc, ModuleOp moduleOp,
                                           llvm::DenseMap<Value, std::vector<Operation *>> &qubitToOps,
                                           llvm::DenseMap<Value, Value> &resolvedQubitAlias) {
        LogicalResult status = success();

        // Helper to resolve the final canonical Qubit value (transitive alias flattening)
        auto resolve = [&](Value v) -> Value {
            llvm::SmallPtrSet<Value, 4> seen;
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
            SymbolRefAttr symRef = callOp.getCalleeAttr().dyn_cast_or_null<SymbolRefAttr>();
            if (!symRef) {
                return WalkResult::advance();
            }

            // Resolve callee definition from symbol table
            Operation *callee = SymbolTable::lookupNearestSymbolFrom(moduleOp, symRef);
            if (!callee || callee->getNumRegions() == 0) {
                return WalkResult::advance();
            }

            Block &entry = callee->getRegion(0).front();

            // Map formal function arguments to actual call operands
            llvm::DenseMap<Value, Value> argMap;
            ValueRange formalArgs = entry.getArguments();
            ValueRange actualArgs = callOp->getOperands();
            for (size_t i = 0, e = std::min(formalArgs.size(), actualArgs.size()); i < e; ++i) {
                argMap[formalArgs[i]] = actualArgs[i];
            }

            // Traverse the callee body and collect MemoryEffect ops (like qinit, epr, measure)
            entry.walk([&](MemoryEffectOpInterface memOp) {
                llvm::SmallVector<MemoryEffects::EffectInstance, 4> effects;
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
            callee->walk([&](netqasm::ReturnOp ret) {
                ValueRange retVals = ret.getOperands();
                auto callResults = callOp->getResults();

                for (size_t i = 0, e = std::min(retVals.size(), callResults.size()); i < e; ++i) {
                    Value result = callResults[i];
                    Value retVal = retVals[i];
                    Value final = resolve(retVal);

                    // If the returned value comes from a QAlloc, record the alias
                    if (auto *def = final.getDefiningOp(); def && isa<netqasm::QAllocOp>(def)) {
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

        return status;
    }

    static void buildMilpQubits(const llvm::DenseMap<Value, std::vector<Operation *>> &qubitToOps,
                                const std::unordered_map<mlir::Operation *, MILPOperation *> &opToMilpOp,
                                std::vector<std::shared_ptr<MILPQubit>> &outQubits) {
        int qubitIndex = 0;

        // Construct MILPQubit objects
        // - Extract alloc & meas ops from qubit usage
        // - Create MILPQubit and attach relevant operations
        for (const auto &entry : qubitToOps) {
            const std::vector<Operation *> &ops = entry.second;

            std::string id = "q" + std::to_string(qubitIndex++);
            std::shared_ptr<MILPQubit> qubitPtr = std::make_shared<MILPQubit>(id);

            MILPOperation *allocOp = nullptr;
            MILPOperation *measOp = nullptr;

            for (Operation *op : ops) {
                if (llvm::isa<netqasm::QInitOp>(op) || llvm::isa<netqasm::EprsOp>(op)) {
                    auto it = opToMilpOp.find(op);
                    allocOp = (it != opToMilpOp.end()) ? it->second : nullptr;
                }

                if (llvm::isa<netqasm::MeasureOp>(op)) {
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

            outQubits.push_back(std::move(qubitPtr));
        }
    }

    std::tuple<std::vector<std::shared_ptr<MILPBlock>>, std::vector<std::shared_ptr<MILPQubit>>, BlockPrecedenceList,
               LogicalResult>
    buildMILPFromMLIR(ModuleOp moduleOp) {
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
        std::vector<std::shared_ptr<MILPBlock>> blocks;
        std::vector<std::shared_ptr<MILPQubit>> qubits;
        BlockPrecedenceList precedences;

        auto mainFuncs = moduleOp.getOps<qoalahost::MainFuncOp>();
        if (mainFuncs.empty()) {
            emitError(moduleOp.getLoc(), "No main function found in module");
            return {blocks, qubits, precedences, failure()};
        }
        qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();

        llvm::StringMap<Operation *> routineMap = collectRoutineMap(moduleOp);

        llvm::StringMap<MILPBlock *> idToBlockMap;
        std::unordered_map<Operation *, MILPOperation *> opToMilpOp;
        std::vector<std::pair<std::string, std::string>> unresolvedEdges;

        if (failed(buildMilpBlocks(mainFunc, routineMap, blocks, precedences, idToBlockMap, opToMilpOp,
                                   unresolvedEdges))) {
            return {blocks, qubits, precedences, failure()};
        }

        // Fail if unresolved edges remain
        if (!unresolvedEdges.empty()) {
            emitError(moduleOp.getLoc(), "Unresolved precedence edges detected");
            return {blocks, qubits, precedences, failure()};
        }

        LLVM_DEBUG({
            llvm::dbgs() << "[MILP] Blocks and tasks:\n";
            for (const auto &bp : blocks) {
                const auto *b = bp.get();
                llvm::dbgs() << " - Block " << b->getId() << " (type=" << (int) b->getType() << ")\n";
                for (const auto &tPtr : b->getTasks()) {
                    const auto *t = tPtr.get();
                    llvm::dbgs() << "    * Task " << t->getId() << " [Group=" << (int) t->getGroup() << "]\n";
                    for (const auto *op : t->getOperations()) {
                        llvm::dbgs() << "        - " << op->getId() << " => " << op->getOperation()->getName()
                                     << " , duration=" << op->getDuration() << "ns\n";
                    }
                }
            }
        });

        LLVM_DEBUG({
            llvm::dbgs() << "[MILP] Precedence edges:\n";
            for (auto &e : precedences) {
                llvm::dbgs() << "  " << e.first->getId() << " -> " << e.second->getId() << "\n";
            }
        });

        LLVM_DEBUG(llvm::dbgs() << "=== Generating all MILPQubits ===\n");

        // Maps canonicalized Qubit Value to list of ops using it (e.g., qinit, measure, epr)
        llvm::DenseMap<Value, std::vector<Operation *>> qubitToOps;
        // Maps result of call to actual QAlloc op it aliases (transitive resolution)
        llvm::DenseMap<Value, Value> resolvedQubitAlias;

        if (failed(collectQubitUsage(mainFunc, moduleOp, qubitToOps, resolvedQubitAlias))) {
            return {blocks, qubits, precedences, failure()};
        }

        LLVM_DEBUG({
            llvm::dbgs() << "[Qubit→Ops]\n";
            for (auto &entry : qubitToOps) {
                llvm::dbgs() << " - Raw Qubit: " << entry.first << "\n";
                for (auto *op : entry.second) {
                    llvm::dbgs() << "     * " << op->getName() << "\n";
                }
            }
        });

        buildMilpQubits(qubitToOps, opToMilpOp, qubits);

        LLVM_DEBUG({
            llvm::dbgs() << "[MILP] Constructed MILPQubits:\n";
            for (const auto &q : qubits) {
                llvm::dbgs() << " - " << q->getId() << ": alloc=";
                llvm::dbgs() << (q->getAllocation() ? q->getAllocation()->getId() : "null") << ", meas=";
                llvm::dbgs() << (q->getMeasurement() ? q->getMeasurement()->getId() : "null") << "\n";
            }
        });

        return {blocks, qubits, precedences, success()};
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

        // Create the (still empty) problem before any variables are added.
        if (SCIPcreateProbBasic(scip_, "MILP_BlockOrdering") != SCIP_OKAY) {
            return false;
        }
        return true;
    }

    void MILPModelBuilder::setProblemData(const std::vector<std::shared_ptr<MILPBlock>> &blocks,
                                          const std::vector<std::shared_ptr<MILPQubit>> &qubits,
                                          const BlockPrecedenceList precedences) {
        blocks_ = blocks;
        qubits_ = qubits;
        precedences_ = precedences;
    }

    void MILPModelBuilder::createVariables() {
        int total = 0;
        for (const std::shared_ptr<MILPBlock> &blk : blocks_) {
            for (const std::unique_ptr<MILPOperation> &op : blk->getOperations()) {
                total += op->getDuration();
                const std::string &id = op->getId();
                if (!startVars_.count(id)) {
                    startVars_[id] = createVariable("s_" + id, /*strictlyPositive=*/false);
                }
            }
        }
        bigM_ = 2 * total;
        LLVM_DEBUG(llvm::dbgs() << "M=" << bigM_ << "\n");
    }

    void MILPModelBuilder::addIntraTaskOrderingConstraints() {
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
                    int rhs = o1->getDuration();
                    SCIPcreateConsBasicLinear(scip_, &c, name.c_str(), 0, nullptr, nullptr, rhs, rhs);
                    SCIPaddCoefLinear(scip_, c, startVars_[o2->getId()], 1.0);
                    SCIPaddCoefLinear(scip_, c, startVars_[o1->getId()], -1.0);
                    SCIPaddCons(scip_, c);
                    SCIPreleaseCons(scip_, &c);
                }
            }
        }
    }

    void MILPModelBuilder::addBlockPrecedenceConstraints() {
        // Adds precedence constraints between blocks based on their dependency edges.
        // If block A must precede block B, we enforce:
        //      start(firstOp(B)) ≥ start(lastOp(A)) + duration(lastOp(A))
        // This ensures B's operations begin only after A's last operation finishes.
        // Implemented using a linear inequality with a lower bound (right-hand side).

        for (const auto &e : precedences_) {
            const MILPBlock *pred = e.first;
            const MILPBlock *succ = e.second;
            const MILPOperation *predLast = pred->lastOp();
            const MILPOperation *succFirst = succ->firstOp();

            SCIP_CONS *c;
            std::string name = "prec_" + pred->getId() + "_" + succ->getId();
            int lhs = predLast->getDuration();
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

    void MILPModelBuilder::addFCFSTaskConstraints() {
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

        const int eps = 1;

        // Build transitive closure of the precedence DAG (reachable pairs).
        Closure clos;
        for (const auto &e : precedences_) {
            clos.insert({e.first->getId(), e.second->getId()});
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
        const int B = static_cast<int>(blocks_.size());
        for (int i = 0; i < B; ++i) {
            const MILPBlock *b1 = blocks_[i].get();
            for (int j = i + 1; j < B; ++j) {
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
                const std::vector<int> idx =
                        (t1.size() == 3 && t2.size() == 3) ? std::vector<int>{0, 1, 2} : std::vector<int>{0};
                for (int k : idx) {
                    const MILPOperation *o1 = t1[k]->getOperations().front();
                    const MILPOperation *o2 = t2[k]->getOperations().front();
                    int dur1 = 0, dur2 = 0;
                    for (MILPOperation *op : t1[k]->getOperations()) {
                        dur1 += op->getDuration();
                    }
                    for (MILPOperation *op : t2[k]->getOperations()) {
                        dur2 += op->getDuration();
                    }

                    // //  FCFS inequality 1: s(o2) - s(o1) + M z ≥ dur1+eps
                    {
                        SCIP_CONS *c;
                        std::string n = "fcfs1_" + zname + "_" + std::to_string(k);
                        int lhs = dur1 + eps;
                        SCIPcreateConsBasicLinear(scip_, &c, n.c_str(), 0, nullptr, nullptr, lhs, SCIPinfinity(scip_));
                        SCIPaddCoefLinear(scip_, c, startVars_[o2->getId()], 1.0);
                        SCIPaddCoefLinear(scip_, c, startVars_[o1->getId()], -1.0);
                        SCIPaddCoefLinear(scip_, c, z, bigM_);
                        SCIPaddCons(scip_, c);
                        SCIPreleaseCons(scip_, &c);
                    }
                    // FCFS inequality 2: s(o1) - s(o2) - M z ≥ dur2+eps - M
                    {
                        SCIP_CONS *c;
                        std::string n = "fcfs2_" + zname + "_" + std::to_string(k);
                        int lhs = dur2 + eps - bigM_;
                        SCIPcreateConsBasicLinear(scip_, &c, n.c_str(), 0, nullptr, nullptr, lhs, SCIPinfinity(scip_));
                        SCIPaddCoefLinear(scip_, c, startVars_[o1->getId()], 1.0);
                        SCIPaddCoefLinear(scip_, c, startVars_[o2->getId()], -1.0);
                        SCIPaddCoefLinear(scip_, c, z, -bigM_);
                        SCIPaddCons(scip_, c);
                        SCIPreleaseCons(scip_, &c);
                    }
                }
                SCIPreleaseVar(scip_, &z);
            }
        }
    }

    void MILPModelBuilder::addIntraBlockSequencingConstraints() {
        // Enforces intra-block task precedence constraints for QL/QC blocks that follow the standard
        // 3-task structure: Task1 → Task2 → Task3.
        // For each such block:
        //  - Task2 must start *after* Task1 finishes
        //  - Task3 must start *after* Task2 finishes
        // These are encoded as inequality constraints:
        //      start(t2) ≥ start(t1) + dur(t1)
        //      start(t3) ≥ start(t2) + dur(t2)

        for (const std::shared_ptr<MILPBlock> &blk : blocks_) {
            if (blk->getType() != OpType::QL && blk->getType() != OpType::QC) {
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

            int dur1 = 0, dur2 = 0;
            for (MILPOperation *op : t1->getOperations()) {
                dur1 += op->getDuration();
            }
            for (MILPOperation *op : t2->getOperations()) {
                dur2 += op->getDuration();
            }

            // t2 after t1
            {
                SCIP_CONS *c;
                std::string n = "seq1_" + blk->getId();
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
                SCIPcreateConsBasicLinear(scip_, &c, n.c_str(), 0, nullptr, nullptr, dur2, dur2);
                SCIPaddCoefLinear(scip_, c, startVars_[first3->getId()], 1.0);
                SCIPaddCoefLinear(scip_, c, startVars_[first2->getId()], -1.0);
                SCIPaddCons(scip_, c);
                SCIPreleaseCons(scip_, &c);
            }
        }
    }

    void MILPModelBuilder::setObjective() {
        // Sets the MILP objective to minimize the total qubit usage time across all qubits.
        // For each qubit:
        //  - MeasureOp start time contributes positively to the objective
        //  - AllocationOp start time contributes negatively
        // This models:
        //      Objective = sum ((start(Meas) + dur(meas)) - (start(Alloc) - dur(alloc)))
        //                = Total lifetime time per qubit
        // In this implementation we remove dur(meas) - dur(alloc) from the objective, as those are not
        // variables and have no influence on the result.
        SCIPsetObjsense(scip_, qoalaOptUnoptimize ? SCIP_OBJSENSE_MAXIMIZE : SCIP_OBJSENSE_MINIMIZE);

        for (const std::shared_ptr<MILPQubit> &q : qubits_) {
            const MILPOperation *alloc = q->getAllocation();
            const MILPOperation *meas = q->getMeasurement();
            if (!(alloc && meas)) {
                continue;
            }

            SCIPchgVarObj(scip_, startVars_[alloc->getId()], -1.0);
            SCIPchgVarObj(scip_, startVars_[meas->getId()], 1.0);
        }
    }

    bool MILPModelBuilder::optimize() { return (SCIPsolve(scip_) == SCIP_OKAY); }

    double MILPModelBuilder::getOperationStartTime(const std::string &id) const {
        // Returns the computed start time (from SCIP solution) for a given operation by ID.
        // Includes defensive checks and debug warnings for missing or invalid SCIP variable values.

        auto it = startVars_.find(id);
        if (it == startVars_.end()) {
            LLVM_DEBUG(llvm::dbgs() << "Warning: start time requested for unknown operation " << id << "\n");
            return -1.0;
        }

        double val = SCIPgetSolVal(scip_, nullptr, it->second);

        // Defensive checks for invalid SCIP values
        if (val < 0.0 || std::isnan(val) || std::isinf(val)) {
            LLVM_DEBUG(llvm::dbgs() << "Warning: invalid start time for operation " << id << ": " << val << "\n");
        }

        return val;
    }

    void MILPModelBuilder::cleanup() {
        for (std::pair<const std::string, SCIP_VAR *> &entry : startVars_) {
            SCIPreleaseVar(scip_, &entry.second);
        }
        startVars_.clear();

        if (scip_ != nullptr) {
            SCIPfree(&scip_);
            scip_ = nullptr;
        }
    }

    SCIP_VAR *MILPModelBuilder::createVariable(const std::string &name, bool strictlyPositive) {
        double lb = strictlyPositive ? 1.0 : 0.0;
        SCIP_VAR *v = nullptr;
        SCIPcreateVarBasic(scip_, &v, name.c_str(), lb, SCIPinfinity(scip_), 0.0, SCIP_VARTYPE_INTEGER);
        SCIPaddVar(scip_, v);
        return v;
    }

    std::vector<std::string> MILPModelBuilder::getOrderedBlocks() {
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

            const reordering::MILPOperation *firstOp = ops.front().get();
            const reordering::MILPOperation *lastOp = ops.back().get();
            double start = getOperationStartTime(firstOp->getId());
            double end = getOperationStartTime(lastOp->getId()) + lastOp->getDuration();

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
        for (const auto &[id, _, __] : blockTimes) {
            orderedBlockIds.push_back(id);
        }

        return orderedBlockIds;
    }

    LogicalResult reorderBlocksByMilpOrder(ModuleOp moduleOp, const std::vector<std::string> &orderedIds) {
        auto mainFuncs = moduleOp.getOps<qoalahost::MainFuncOp>();
        if (mainFuncs.empty()) {
            emitError(moduleOp.getLoc(), "No main function found in module");
            return failure();
        }
        qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();
        auto &body = mainFunc.getBody();

        llvm::StringMap<Block *> idToBlock;
        Block *returnBlock = nullptr;

        // Reorder the blocks in-place in the function body based on the MILP-provided order.
        // And identify any block containing a ReturnOp.
        for (Block &blk : body) {
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
        Block *insertionPoint = &body.front();
        for (const std::string &id : orderedIds) {
            auto it = idToBlock.find(id);
            if (it == idToBlock.end()) {
                return moduleOp.emitError("unknown block_id \"") << id << "\"", failure();
            }

            Block *blk = it->second;
            if (blk != insertionPoint) {
                blk->moveBefore(insertionPoint);
            }
            insertionPoint = blk->getNextNode();
        }

        // Ensure returnBlock (if present) is the final block in the function body
        if (returnBlock && returnBlock->getNextNode() != nullptr) {
            returnBlock->moveBefore(nullptr);
        } // Moves to the end of the block list

        return success();
    }
} // namespace qoala::analysis::reordering
