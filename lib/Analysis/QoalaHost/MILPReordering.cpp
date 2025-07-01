#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "llvm/Support/Debug.h"
#include "mlir/IR/SymbolTable.h"
#include "Dialect/QoalaHost/Passes.h"
#include "llvm/Support/FormatVariadic.h"
#include "Tools/QoalaOpt.h"

#include "scip/scip.h"
#include "scip/scipdefplugins.h"

#define DEBUG_TYPE "qoalahost-reorder-blocks-pass-internal"

using namespace mlir;
using namespace qoala::dialects;
using namespace qoala::analysis;

namespace qoala::analysis::reordering {
    double getOperationDuration(Operation *op) {
        // CallOp and NopOp are the Pre and Post tasks
        if (isa<qoalahost::CallOp>(op) || isa<qoalahost::NopOp>(op))
            return qoalaOptHostInstrTime;

        // TODO: check in translation if the size of tensor decides the number of send ops
        if (isa<qoalahost::SendIntsOp>(op) || isa<qoalahost::SendFloatsOp>(op))
            return qoalaOptHostInstrTime;

        // TODO: check in translation if the size of tensor decides the number of recv ops
        if (isa<qoalahost::RecvIntsOp>(op) || isa<qoalahost::RecvFloatsOp>(op))
            return qoalaOptLatency + qoalaOptHostPeerLatency;

        // Netqasm operations that are classical operations
        if (isa<netqasm::QAllocOp>(op) || isa<netqasm::QFreeOp>(op))
            return qoalaOptQNosInstrTime;

        // Each operand will be converted to one netqasm instruction in the iqoala file
        if (isa<netqasm::ReturnOp>(op))
            return op->getNumOperands() * qoalaOptQNosInstrTime;

        // Single qubit gate operations, qubit init and measure
        if (isa<netqasm::QInitOp>(op) || isa<netqasm::RotateXOp>(op) || isa<netqasm::RotateYOp>(op) ||
            isa<netqasm::RotateZOp>(op) || isa<netqasm::HadamardOp>(op) || isa<netqasm::MeasureOp>(op))
            return qoalaOptSingleGateDuration;

        // Two qubits gate oeprations
        if (isa<netqasm::CnotOp>(op) || isa<netqasm::CzOp>(op) || isa<netqasm::CrotXOp>(op))
            return qoalaOptTwoGateDuration;

        if (isa<netqasm::EprsOp>(op))
            return qoalaOptLinkDuration;

        if (isa<netqasm::EprsMeasureOp>(op))
            return qoalaOptLinkDuration + qoalaOptSingleGateDuration;

        // Any other instructions are the ones in CL blocks
        return qoalaOptHostInstrTime;
    }

    OpType getBlockType(Operation *op, ModuleOp moduleOp) {
        // First, handle communication ops (CC)
        if (llvm::isa<qoalahost::SendIntsOp>(op) || llvm::isa<qoalahost::RecvIntsOp>(op) ||
            llvm::isa<qoalahost::SendFloatsOp>(op) || llvm::isa<qoalahost::RecvFloatsOp>(op)) {
            return OpType::CC;
        }

        // Then check for call-based classification
        if (auto callOp = llvm::dyn_cast<qoalahost::CallOp>(op)) {
            SymbolRefAttr symRef = callOp.getCalleeAttr().dyn_cast_or_null<SymbolRefAttr>();
            if (!symRef)
                return OpType::CL;
            Operation *callee = SymbolTable::lookupNearestSymbolFrom(moduleOp, symRef);
            if (llvm::isa<netqasm::RequestRoutineOp>(callee))
                return OpType::QC;
            if (llvm::isa<netqasm::LocalRoutineOp>(callee))
                return OpType::QL;
        }

        // Default fallback
        return OpType::CL;
    }


    LogicalResult createTasksForBlock(MILPBlock *blk, const Location &loc) {
        const std::vector<MILPOperation *> &ops = blk->getOperations();
        if (ops.empty()) {
            emitError(loc) << "MILPBlock '" << blk->getId() << "' has no operations — this should not happen.";
            return failure();
        }

        switch (blk->getType()) {
            case OpType::CL:
            case OpType::CC: {
                if (blk->getType() == OpType::CC && ops.size() != 1) {
                    emitError(loc) << "MILPBlock of type CC should contain exactly one operation, but got "
                                   << ops.size();
                    return failure();
                }
                std::unique_ptr<MILPTask> task = std::make_unique<MILPTask>("0", blk, TaskGroup::C);
                for (MILPOperation *op: ops)
                    task->addOperation(op);
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
                t0->addOperation(ops.front());
                blk->addTask(std::move(t0));
                // Task 1 – middle (Q): Quantum Routine
                std::unique_ptr<MILPTask> t1 = std::make_unique<MILPTask>("1", blk, TaskGroup::Q);
                for (size_t i = 1; i + 1 < ops.size(); ++i)
                    t1->addOperation(ops[i]);
                blk->addTask(std::move(t1));
                // Task 2 – return (C) : PostTask.
                std::unique_ptr<MILPTask> t2 = std::make_unique<MILPTask>("2", blk, TaskGroup::C);
                t2->addOperation(ops.back());
                blk->addTask(std::move(t2));
                break;
            }
        }
        return success();
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
        //      - Handle `netqasm.return` instructions in inlined routines by inserting a `qoalahost.nop` operation in
        //      the caller block to explicitly represent a PostTask from the qoala runtime model.
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

        llvm::StringMap<MILPBlock *> idToBlockMap;
        std::unordered_map<Operation *, MILPOperation *> opToMilpOp;
        std::vector<std::pair<std::string, std::string>> unresolvedEdges;

        auto mainFuncs = moduleOp.getOps<qoalahost::MainFuncOp>();
        if (mainFuncs.empty()) {
            emitError(moduleOp.getLoc(), "No main function found in module");
            return {blocks, qubits, precedences, failure()};
        }
        qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();

        // TODO: do not use SmallPtrSet because we have no way of knowing the needed size
        llvm::SmallPtrSet<Block *, 32> visitedBlocks;
        LogicalResult status = success();

        // This walk processes each `qoalahost.BlkMeta` operation in the `mainFunc`
        // body exactly once (per MLIR block) to construct the high-level MILP structure.
        mainFunc.walk([&](qoalahost::BlkMeta blkMeta) {
            if (failed(status))
                return;

            Block *block = blkMeta->getBlock();
            if (!visitedBlocks.insert(block).second)
                return;

            // Avoid processing the same MLIR block twice (only one BlkMeta per block is valid)
            Block::iterator firstIt = std::next(block->begin());
            if (firstIt == block->end()) {
                emitError(blkMeta.getLoc(), "Block has no body after BlkMeta");
                status = failure();
                return;
            }
            Operation *firstOp = &*firstIt;

            OpType blkType = getBlockType(firstOp, moduleOp);

            // Create new MILPBlock object and associate with block ID and type
            std::string blkId = blkMeta.getBlockId().str();
            std::shared_ptr<MILPBlock> blkPtr = std::make_shared<MILPBlock>(blkId, blkType);
            blkPtr->setBlock(block);
            MILPBlock *blk = blkPtr.get();
            idToBlockMap[blkId] = blk;

            // Index operations to create unique IDs for MILPOperation
            int opIdx = 0;

            // Walk through the actual instructions in the MLIR block
            for (Block::iterator it = firstIt; it != block->end(); ++it) {
                Operation *op = &*it;

                // Stop early if we reach a return/nop_term in a classical block
                if (blkType == OpType::CL && (llvm::isa<qoalahost::ReturnOp>(op) || llvm::isa<qoalahost::NopTOp>(op)))
                    break;

                // Create MILPOperation
                std::string opId = blkId + "::" + std::to_string(opIdx++);
                MILPOperation *milpOp = new MILPOperation(opId, blkType, getOperationDuration(op));
                milpOp->setOperation(op);
                blk->addOperation(milpOp);
                opToMilpOp[op] = milpOp;

                // Handle inlining for quantum-local or quantum-communication blocks
                // - Inline body of the called routine
                // - Track operations
                // - Add a qoalahost.nop at the end to model post-task transition
                if (blkType == OpType::QL || blkType == OpType::QC) {
                    if (auto callOp = llvm::dyn_cast<qoalahost::CallOp>(op)) {
                        SymbolRefAttr sym = callOp.getCalleeAttr().dyn_cast_or_null<SymbolRefAttr>();
                        Operation *callee = sym ? SymbolTable::lookupNearestSymbolFrom(moduleOp, sym) : nullptr;
                        auto calleeFunc = llvm::dyn_cast_or_null<FunctionOpInterface>(callee);
                        if (!calleeFunc) {
                            emitError(op->getLoc(), "Callee is not a FunctionOpInterface");
                            status = failure();
                            return;
                        }

                        bool foundReturn = false;

                        // Inline all ops from callee body
                        for (Block &cb: calleeFunc->getRegion(0)) {
                            for (Operation &cOp: cb) {
                                std::string subId = blkId + "::" + std::to_string(opIdx++);
                                MILPOperation *milpSub = new MILPOperation(subId, blkType, getOperationDuration(&cOp));
                                milpSub->setOperation(&cOp);
                                blk->addOperation(milpSub);
                                opToMilpOp[&cOp] = milpSub;
                                if (llvm::isa<netqasm::ReturnOp>(cOp)) {
                                    // Insert qoalahost.NopOp in caller block & track to model PostTask
                                    OpBuilder b(moduleOp.getContext());
                                    b.setInsertionPointToEnd(block);
                                    qoalahost::NopOp nop = b.create<qoalahost::NopOp>(cOp.getLoc());

                                    std::string nopId = blkId + "::" + std::to_string(opIdx++);
                                    MILPOperation *milpNop =
                                            new MILPOperation(nopId, blkType, getOperationDuration(nop.getOperation()));
                                    milpNop->setOperation(nop.getOperation());
                                    blk->addOperation(milpNop);
                                    opToMilpOp[nop.getOperation()] = milpNop;

                                    foundReturn = true;
                                    break;
                                }
                            }
                            if (foundReturn)
                                break;
                        }
                        if (!foundReturn) {
                            emitError(op->getLoc(), "Callee does not end in netqasm::ReturnOp");
                            status = failure();
                        }
                        break; // Only one call is handled per block
                    }
                }

                // CC blocks contain a single communication operation only
                if (blkType == OpType::CC)
                    break;
            }

            if (failed(status))
                return;

            // Record precedence edges from block attributes. For our optimization all types of precedences are
            // equivalent. The differenciation is needed for the translation step.
            auto recordEdge = [&](StringRef predId) {
                if (predId.empty())
                    return;
                auto it = idToBlockMap.find(predId);
                if (it != idToBlockMap.end())
                    precedences.emplace_back(it->second, blk);
                else
                    unresolvedEdges.emplace_back(predId.str(), blkId);
            };
            if (ArrayAttr a = blkMeta.getPredecessorsAttr()) {
                for (llvm::StringRef s: a.getAsValueRange<StringAttr>()) {
                    recordEdge(s);
                }
            }
            if (ArrayAttr a = blkMeta.getDependenciesAttr()) {
                for (llvm::StringRef s: a.getAsValueRange<StringAttr>()) {
                    recordEdge(s);
                }
            }
            if (StringAttr a = blkMeta.getPrevEntAttr()) {
                recordEdge(a.getValue());
            }
            if (StringAttr a = blkMeta.getPrevCommAttr()) {
                recordEdge(a.getValue());
            }

            // Generate tasks for this MILP block (task-level subdivision)
            if (failed(createTasksForBlock(blk, blkMeta.getLoc()))) {
                status = failure();
                return;
            }

            blocks.push_back(std::move(blkPtr));
        });

        if (failed(status))
            return {blocks, qubits, precedences, failure()};

        // Fail if unresolved edges remain
        if (!unresolvedEdges.empty()) {
            emitError(moduleOp.getLoc(), "Unresolved precedence edges detected");
            return {blocks, qubits, precedences, failure()};
        }

        LLVM_DEBUG({
            llvm::dbgs() << "[MILP] Blocks and tasks:\n";
            for (const auto &bp: blocks) {
                const auto *b = bp.get();
                llvm::dbgs() << " - Block " << b->getId() << " (type=" << (int) b->getType() << ")\n";
                for (const auto &tPtr: b->getTasks()) {
                    const auto *t = tPtr.get();
                    llvm::dbgs() << "    * Task " << t->getId() << " [Group=" << (int) t->getGroup() << "]\n";
                    for (const auto *op: t->getOperations())
                        llvm::dbgs() << "        - " << op->getId() << " => " << op->getOperation()->getName()
                                     << " , duration=" << op->getDuration() << "ns\n";
                }
            }
        });

        LLVM_DEBUG({
            llvm::dbgs() << "[MILP] Precedence edges:\n";
            for (auto &e: precedences)
                llvm::dbgs() << "  " << e.first->getId() << " -> " << e.second->getId() << "\n";
        });

        // TODO: check what happens if we have a netqasm::EprsMeasureOp
        LLVM_DEBUG(llvm::dbgs() << "=== Generating all MILPQubits ===\n");

        // Maps canonicalized Qubit Value to list of ops using it (e.g., qinit, measure, epr)
        llvm::DenseMap<Value, std::vector<Operation *>> qubitToOps;
        // Maps result of call to actual QAlloc op it aliases (transitive resolution)
        llvm::DenseMap<Value, Value> resolvedQubitAlias;

        // Helper to resolve the final canonical Qubit value (transitive alias flattening)
        auto resolve = [&](Value v) -> Value {
            llvm::SmallPtrSet<Value, 4> seen;
            while (resolvedQubitAlias.count(v)) {
                if (!seen.insert(v).second)
                    break;
                v = resolvedQubitAlias[v];
            }
            return v;
        };

        // Walk all CallOps, analyze their callee routines
        //  - Collect MemoryEffect ops inside the inlined body
        //  - Track any returned qubit (e.g. %0 = call @foo) that aliases a QAlloc
        mainFunc.walk([&](qoalahost::CallOp callOp) -> WalkResult {
            SymbolRefAttr symRef = callOp.getCalleeAttr().dyn_cast_or_null<SymbolRefAttr>();
            if (!symRef)
                return WalkResult::advance();

            // Resolve callee definition from symbol table
            Operation *callee = SymbolTable::lookupNearestSymbolFrom(moduleOp, symRef);
            if (!callee || callee->getNumRegions() == 0)
                return WalkResult::advance();

            Block &entry = callee->getRegion(0).front();

            // Map formal function arguments to actual call operands
            llvm::DenseMap<Value, Value> argMap;
            ValueRange formalArgs = entry.getArguments();
            ValueRange actualArgs = callOp->getOperands();
            for (size_t i = 0, e = std::min(formalArgs.size(), actualArgs.size()); i < e; ++i)
                argMap[formalArgs[i]] = actualArgs[i];

            // Traverse the callee body and collect MemoryEffect ops (like qinit, epr, measure)
            entry.walk([&](MemoryEffectOpInterface memOp) {
                llvm::SmallVector<MemoryEffects::EffectInstance, 4> effects;
                memOp.getEffects(effects);

                for (MemoryEffects::EffectInstance &eff: effects) {
                    Value v = eff.getValue();
                    if (!v)
                        continue;

                    // Resolve value through argument mapping
                    Value resolved = argMap.lookup(v);
                    if (!resolved)
                        resolved = v;

                    // Canonicalize through aliases (if applicable)
                    Value canonical = resolve(resolved);
                    qubitToOps[canonical].push_back(memOp.getOperation());

                    LLVM_DEBUG(llvm::dbgs() << "    [Track] " << memOp->getName() << " → " << canonical << '\n');
                }
            });

            // If call has no result, skip alias tracking
            if (callOp->getNumResults() == 0)
                return WalkResult::advance();

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
                emitError(callOp.getLoc(), "callee has no netqasm.return instruction");
                return WalkResult::interrupt();
            }

            return WalkResult::advance();
        });

        LLVM_DEBUG({
            llvm::dbgs() << "[Qubit→Ops]\n";
            for (auto &entry: qubitToOps) {
                llvm::dbgs() << " - Raw Qubit: " << entry.first << "\n";
                for (auto *op: entry.second) {
                    llvm::dbgs() << "     * " << op->getName() << "\n";
                }
            }
        });

        int qubitIndex = 0;

        // Construct MILPQubit objects
        // - Extract alloc & meas ops from qubit usage
        // - Create MILPQubit and attach relevant operations
        for (const auto &entry: qubitToOps) {
            const std::vector<Operation *> &ops = entry.second;

            std::string id = "q" + std::to_string(qubitIndex++);
            std::shared_ptr<MILPQubit> qubitPtr = std::make_shared<MILPQubit>(id);

            MILPOperation *allocOp = nullptr;
            MILPOperation *measOp = nullptr;

            for (Operation *op: ops) {
                if (llvm::isa<netqasm::QInitOp>(op) || llvm::isa<netqasm::EprsOp>(op)) {
                    allocOp = opToMilpOp.count(op) ? opToMilpOp[op] : nullptr;
                }

                if (llvm::isa<netqasm::MeasureOp>(op)) {
                    measOp = opToMilpOp.count(op) ? opToMilpOp[op] : nullptr;
                }
            }

            // Attach known alloc/meas to the qubit model object
            if (allocOp)
                qubitPtr->setAllocation(allocOp);
            if (measOp)
                qubitPtr->setMeasurement(measOp);

            LLVM_DEBUG(llvm::dbgs() << "  [Qubit] " << id << ": alloc=" << (allocOp ? allocOp->getId() : "null")
                                    << ", meas=" << (measOp ? measOp->getId() : "null") << "\n");

            qubits.push_back(std::move(qubitPtr));
        }

        LLVM_DEBUG({
            llvm::dbgs() << "[MILP] Constructed MILPQubits:\n";
            for (const auto &q: qubits) {
                llvm::dbgs() << " - " << q->getId() << ": alloc=";
                llvm::dbgs() << (q->getAllocation() ? q->getAllocation()->getId() : "null") << ", meas=";
                llvm::dbgs() << (q->getMeasurement() ? q->getMeasurement()->getId() : "null") << "\n";
            }
        });

        return {blocks, qubits, precedences, success()};
    }

    bool MILPModelBuilder::initialize() {
        if (SCIPcreate(&scip_) != SCIP_OKAY)
            return false;
        if (SCIPincludeDefaultPlugins(scip_) != SCIP_OKAY)
            return false;
        /* Create the (still empty) problem before any variables are added. */
        if (SCIPcreateProbBasic(scip_, "MILP_BlockOrdering") != SCIP_OKAY)
            return false;
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
        double total = 0.0;
        for (const std::shared_ptr<MILPBlock> &blk: blocks_)
            for (const MILPOperation *op: blk->getOperations()) {
                total += op->getDuration();
                const std::string &id = op->getId();
                if (!startVars_.count(id))
                    startVars_[id] = createContinuousVariable("s_" + id, 0.0, SCIPinfinity(scip_));
            }
        bigM_ = 2 * total;

        // Add offset here if needed (safe stage)
        if (constOffset_ != 0.0)
            SCIPaddObjoffset(scip_, constOffset_);
    }

    // equality chain inside each task
    void MILPModelBuilder::addIntraTaskOrderingConstraints() {
        for (const std::shared_ptr<MILPBlock> &blk: blocks_) {
            for (const std::unique_ptr<MILPTask> &t: blk->getTasks()) {
                const std::vector<MILPOperation *> &ops = t->getOperations();
                for (size_t j = 0; j + 1 < ops.size(); ++j) {
                    const MILPOperation *o1 = ops[j];
                    const MILPOperation *o2 = ops[j + 1];
                    SCIP_CONS *c;
                    std::string name = "ord_" + o1->getId() + "_" + o2->getId();
                    double rhs = o1->getDuration();
                    SCIPcreateConsBasicLinear(scip_, &c, name.c_str(), 0, nullptr, nullptr, rhs, rhs);
                    SCIPaddCoefLinear(scip_, c, startVars_[o2->getId()], 1.0);
                    SCIPaddCoefLinear(scip_, c, startVars_[o1->getId()], -1.0);
                    SCIPaddCons(scip_, c);
                    SCIPreleaseCons(scip_, &c);
                }
            }
        }
    }
    // precedence edges
    void MILPModelBuilder::addBlockPrecedenceConstraints() {
        for (const auto &e: precedences_) {
            const MILPBlock *pred = e.first;
            const MILPBlock *succ = e.second;
            const MILPOperation *predLast = pred->lastOp();
            const MILPOperation *succFirst = succ->firstOp();

            SCIP_CONS *c;
            std::string name = "prec_" + pred->getId() + "_" + succ->getId();
            double lhs = predLast->getDuration();
            SCIPcreateConsBasicLinear(scip_, &c, name.c_str(), 0, nullptr, nullptr, lhs, SCIPinfinity(scip_));
            SCIPaddCoefLinear(scip_, c, startVars_[succFirst->getId()], 1.0);
            SCIPaddCoefLinear(scip_, c, startVars_[predLast->getId()], -1.0);
            SCIPaddCons(scip_, c);
            SCIPreleaseCons(scip_, &c);
        }
    }

    void MILPModelBuilder::addFCFSTaskConstraints() {
        const double eps = 1.0;

        // transitive closure of precedence DAG
        Closure clos;
        for (const auto &e: precedences_)
            clos.insert({e.first->getId(), e.second->getId()});

        bool grown;
        do {
            grown = false;
            for (const std::pair<std::string, std::string> &p: clos)
                for (const std::pair<std::string, std::string> &q: clos)
                    if (p.second == q.first && clos.insert({p.first, q.second}).second)
                        grown = true;
        } while (grown);

        const int B = static_cast<int>(blocks_.size());
        for (int i = 0; i < B; ++i) {
            const MILPBlock *b1 = blocks_[i].get();
            for (int j = i + 1; j < B; ++j) {
                const MILPBlock *b2 = blocks_[j].get();

                if (reachable(b1, b2, clos) || reachable(b2, b1, clos))
                    continue;
                if (b1->getTasks().empty() || b2->getTasks().empty())
                    continue;
                if (b1->getTasks().front()->getGroup() != b2->getTasks().front()->getGroup())
                    continue;

                // binary selector z
                SCIP_VAR *z;
                std::string zname = "z_" + b1->getId() + "_" + b2->getId();
                SCIPcreateVarBasic(scip_, &z, zname.c_str(), 0.0, 1.0, 0.0, SCIP_VARTYPE_BINARY);
                SCIPaddVar(scip_, z);

                const std::vector<std::unique_ptr<MILPTask>> &t1 = b1->getTasks();
                const std::vector<std::unique_ptr<MILPTask>> &t2 = b2->getTasks();
                const std::vector<int> idx =
                        (t1.size() == 3 && t2.size() == 3) ? std::vector<int>{0, 1, 2} : std::vector<int>{0};
                for (int k: idx) {
                    const MILPOperation *o1 = t1[k]->getOperations().front();
                    const MILPOperation *o2 = t2[k]->getOperations().front();
                    double dur1 = 0.0, dur2 = 0.0;
                    for (MILPOperation *op: t1[k]->getOperations())
                        dur1 += op->getDuration();
                    for (MILPOperation *op: t2[k]->getOperations())
                        dur2 += op->getDuration();

                    // s(o2) - s(o1) + M z ≥ dur1+eps
                    {
                        SCIP_CONS *c;
                        std::string n = "fcfs1_" + zname + "_" + std::to_string(k);
                        double lhs = dur1 + eps;
                        SCIPcreateConsBasicLinear(scip_, &c, n.c_str(), 0, nullptr, nullptr, lhs, SCIPinfinity(scip_));
                        SCIPaddCoefLinear(scip_, c, startVars_[o2->getId()], 1.0);
                        SCIPaddCoefLinear(scip_, c, startVars_[o1->getId()], -1.0);
                        SCIPaddCoefLinear(scip_, c, z, bigM_);
                        SCIPaddCons(scip_, c);
                        SCIPreleaseCons(scip_, &c);
                    }
                    // s(o1) - s(o2) - M z ≥ dur2+eps - M
                    {
                        SCIP_CONS *c;
                        std::string n = "fcfs2_" + zname + "_" + std::to_string(k);
                        double lhs = dur2 + eps - bigM_;
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
        for (const std::shared_ptr<MILPBlock> &blk: blocks_) {
            if (blk->getType() != OpType::QL && blk->getType() != OpType::QC)
                continue;
            const std::vector<std::unique_ptr<MILPTask>> &tasks = blk->getTasks();
            if (tasks.size() != 3)
                continue;

            const MILPTask *t1 = tasks[0].get();
            const MILPTask *t2 = tasks[1].get();
            const MILPTask *t3 = tasks[2].get();

            const MILPOperation *first2 = t2->getOperations().front();
            const MILPOperation *first3 = t3->getOperations().front();

            double dur1 = 0.0, dur2 = 0.0;
            for (MILPOperation *op: t1->getOperations())
                dur1 += op->getDuration();
            for (MILPOperation *op: t2->getOperations())
                dur2 += op->getDuration();

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
        SCIPsetObjsense(scip_, SCIP_OBJSENSE_MINIMIZE);
        constOffset_ = 0.0;

        for (const std::shared_ptr<MILPQubit> &q: qubits_) {
            const MILPOperation *alloc = q->getAllocation();
            const MILPOperation *meas = q->getMeasurement();
            if (!(alloc && meas))
                continue;

            SCIPchgVarObj(scip_, startVars_[meas->getId()], 1.0);
            SCIPchgVarObj(scip_, startVars_[alloc->getId()], -1.0);
            constOffset_ += meas->getDuration() - alloc->getDuration();
        }
    }

    bool MILPModelBuilder::optimize() { return (SCIPsolve(scip_) == SCIP_OKAY); }


    double MILPModelBuilder::getOperationStartTime(const std::string &id) const {
        auto it = startVars_.find(id);
        return (it == startVars_.end()) ? -1.0 : SCIPgetSolVal(scip_, nullptr, it->second);
    }


    void MILPModelBuilder::cleanup() {
        for (std::pair<const std::string, SCIP_VAR *> &entry: startVars_)
            SCIPreleaseVar(scip_, &entry.second);
        startVars_.clear();

        if (scip_ != nullptr) {
            SCIPfree(&scip_);
            scip_ = nullptr;
        }
    }


    SCIP_VAR *MILPModelBuilder::createContinuousVariable(const std::string &name, double lb, double ub) {
        SCIP_VAR *v = nullptr;
        SCIPcreateVarBasic(scip_, &v, name.c_str(), lb, ub, 0.0, SCIP_VARTYPE_CONTINUOUS);
        SCIPaddVar(scip_, v);
        return v;
    }

    std::vector<std::string> MILPModelBuilder::getOrderedBlocks() {
        std::vector<std::tuple<std::string, double, double>> blockTimes;

        for (const auto &block: blocks_) {
            const auto &ops = block->getOperations();
            if (ops.empty())
                continue;

            const reordering::MILPOperation *firstOp = ops.front();
            const reordering::MILPOperation *lastOp = ops.back();
            double start = getOperationStartTime(firstOp->getId());
            double end = getOperationStartTime(lastOp->getId()) + lastOp->getDuration();

            blockTimes.emplace_back(block->getId(), start, end);
        }

        // Sort by start time
        std::sort(blockTimes.begin(), blockTimes.end(),
                  [](const auto &a, const auto &b) { return std::get<1>(a) < std::get<1>(b); });

        for (const auto &[id, start, end]: blockTimes) {
            LLVM_DEBUG(llvm::dbgs() << "Block " << id << ": [" << start << ", " << end << "]\n");
        }

        std::vector<std::string> orderedBlockIds;
        for (const auto &[id, _, __]: blockTimes) {
            orderedBlockIds.push_back(id);
        }

        return orderedBlockIds;
    }

    LogicalResult reorderBlocksByMilpOrder(ModuleOp moduleOp, const std::vector<std::string> &orderedIds) {
        // TODO: check what happens if we have a qoalahost.return op in its own block at the end of representation
        auto mainFuncs = moduleOp.getOps<qoalahost::MainFuncOp>();
        if (mainFuncs.empty()) {
            emitError(moduleOp.getLoc(), "No main function found in module");
            return failure();
        }
        qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();
        auto &body = mainFunc.getBody();

        llvm::StringMap<Block *> idToBlock;

        for (Block &blk: body) {
            for (Operation &op: blk) {
                if (auto meta = llvm::dyn_cast<qoalahost::BlkMeta>(op)) {
                    idToBlock[meta.getBlockId()] = &blk;
                    break;
                }
            }
        }

        Block *insertionPoint = &body.front();
        for (const std::string &id: orderedIds) {
            auto it = idToBlock.find(id);
            if (it == idToBlock.end())
                return moduleOp.emitError("unknown block_id “") << id << "”", failure();

            Block *blk = it->second;
            if (blk != insertionPoint)
                blk->moveBefore(insertionPoint);
            insertionPoint = blk->getNextNode();
        }

        return success();
    }
} // namespace qoala::analysis::reordering
