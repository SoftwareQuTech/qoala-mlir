#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "llvm/Support/Debug.h"
#include "mlir/IR/SymbolTable.h"
#include "Dialect/QoalaHost/Passes.h"
#include "llvm/Support/FormatVariadic.h"

#include "scip/scip.h"
#include "scip/scipdefplugins.h"

#define DEBUG_TYPE "qoalahost-reorder-blocks-pass-internal"

using namespace mlir;
using namespace qoala::dialects;
using namespace qoala::analysis;

namespace qoala::analysis::reordering {
    std::tuple<std::vector<std::shared_ptr<MILPBlock>>, std::vector<std::shared_ptr<MILPQubit>>, BlockPrecedenceList,
               mlir::LogicalResult>
    buildMILPFromMLIR(mlir::ModuleOp moduleOp) {
        std::vector<std::shared_ptr<MILPBlock>> blocks;
        std::vector<std::shared_ptr<MILPQubit>> qubits;
        std::vector<reordering::BlockPrecedence> precedences;

        std::unordered_map<mlir::Operation *, MILPOperation *> opToMilpOp;

        const auto mainFuncs = moduleOp.getOps<qoalahost::MainFuncOp>();
        if (mainFuncs.empty()) {
            mlir::emitError(moduleOp.getLoc(), "No main function found in module");
            return {blocks, qubits, precedences, failure()};
        }

        qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();

        std::unordered_map<std::string, reordering::MILPBlock *> idToBlockMap;

        LLVM_DEBUG(llvm::dbgs() << "=== Generating all MILP Blocks ===\n");
        for (mlir::Block &block: mainFunc.getBody()) {
            auto blkMeta = llvm::dyn_cast<qoalahost::BlkMeta>(&block.front());
            if (!blkMeta) {
                mlir::emitError(block.front().getLoc(), "Expected BlkMeta as first op in block");
                return {blocks, qubits, precedences, failure()};
            }

            std::string blockId = blkMeta.getBlockId().str();
            reordering::OpType blockType = OpType::CL; // default

            auto blockPtr = std::make_shared<MILPBlock>(blockId, blockType);
            blockPtr->setBlock(&block);

            int opIndex = 0;

            auto it = block.begin();
            ++it; // skip BlkMeta

            if (it == block.end()) {
                mlir::emitError(
                        it->getLoc(),
                        "BlkMeta cannot be the last operation of a block. It does not have the terminator trait.");
                return {blocks, qubits, precedences, failure()};
            }

            mlir::Operation *op = &*it;

            // --- Case 1: CallOp ---
            if (auto callOp = llvm::dyn_cast<qoalahost::CallOp>(op)) {
                auto symRef = callOp.getCalleeAttr().dyn_cast_or_null<mlir::SymbolRefAttr>();
                if (!symRef) {
                    mlir::emitError(op->getLoc(), "CallOp without SymbolRefAttr");
                    return {blocks, qubits, precedences, failure()};
                }

                auto callee = mlir::SymbolTable::lookupNearestSymbolFrom(moduleOp, symRef);
                auto calleeFunc = llvm::dyn_cast_or_null<mlir::FunctionOpInterface>(callee);
                if (!calleeFunc) {
                    mlir::emitError(op->getLoc(), "Callee is not a FunctionOpInterface");
                    return {blocks, qubits, precedences, failure()};
                }

                if (llvm::isa<netqasm::RequestRoutineOp>(calleeFunc))
                    blockType = OpType::QC;
                else if (llvm::isa<netqasm::LocalRoutineOp>(calleeFunc))
                    blockType = OpType::QL;

                blockPtr = std::make_shared<MILPBlock>(blockId, blockType);
                blockPtr->setBlock(&block);

                // Add CallOp itself
                std::string opId = blockId + "::" + std::to_string(opIndex++);
                auto *milpOp = new MILPOperation(opId, blockType, 1.0);
                milpOp->setOperation(op);
                opToMilpOp[op] = milpOp;
                blockPtr->addOperation(milpOp);

                // Add all ops from the callee body
                bool foundReturn = false;
                mlir::Region &calleeRegion = calleeFunc->getRegion(0);
                for (mlir::Block &calleeBlock: calleeRegion) {
                    for (mlir::Operation &calleeOp: calleeBlock) {
                        std::string subOpId = blockId + "::" + std::to_string(opIndex++);
                        auto *milpSubOp = new MILPOperation(subOpId, blockType, 1.0);
                        milpSubOp->setOperation(&calleeOp);
                        opToMilpOp[&calleeOp] = milpSubOp;
                        blockPtr->addOperation(milpSubOp);

                        if (llvm::isa<netqasm::ReturnOp>(calleeOp)) {
                            foundReturn = true;
                            break;
                        }
                    }
                    if (foundReturn)
                        break;
                }

                if (!foundReturn) {
                    mlir::emitError(op->getLoc(), "Callee does not end in netqasm::ReturnOp");
                    return {blocks, qubits, precedences, failure()};
                }

                blocks.push_back(blockPtr);
                continue;
            }

            // --- Case 2: Comm Ops (CC) ---
            if (llvm::isa<qoalahost::SendIntsOp>(op) || llvm::isa<qoalahost::RecvIntsOp>(op) ||
                llvm::isa<qoalahost::SendFloatsOp>(op) || llvm::isa<qoalahost::RecvFloatsOp>(op)) {
                blockType = OpType::CC;
                blockPtr = std::make_shared<MILPBlock>(blockId, blockType);
                blockPtr->setBlock(&block);

                std::string opId = blockId + "::" + std::to_string(opIndex++);
                auto *milpOp = new MILPOperation(opId, blockType, 1.0);
                milpOp->setOperation(op);
                opToMilpOp[op] = milpOp;
                blockPtr->addOperation(milpOp);

                blocks.push_back(blockPtr);
                continue;
            }

            // --- Case 3: Generic CL block ---
            for (; it != block.end(); ++it) {
                mlir::Operation *innerOp = &*it;
                if (llvm::isa<qoalahost::ReturnOp>(innerOp) || llvm::isa<qoalahost::NopTOp>(innerOp))
                    break;

                std::string opId = blockId + "::" + std::to_string(opIndex++);
                auto *milpOp = new MILPOperation(opId, OpType::CL, 1.0);
                milpOp->setOperation(innerOp);
                opToMilpOp[innerOp] = milpOp;
                blockPtr->addOperation(milpOp);
            }

            blocks.push_back(blockPtr);
        }

        LLVM_DEBUG({
            llvm::dbgs() << "[MILP] Finished MILP block construction:\n";
            for (const std::shared_ptr<MILPBlock> &blkPtr: blocks) {
                const MILPBlock &blk = *blkPtr;
                llvm::dbgs() << " - Block " << blk.getId() << " (type=" << static_cast<int>(blk.getType()) << ")\n";
                for (const MILPOperation *milpOp: blk.getOperations()) {
                    mlir::Operation *op = milpOp->getOperation();
                    llvm::StringRef opName = op ? op->getName().getStringRef() : "<null>";
                    llvm::dbgs() << "     * " << milpOp->getId() << " → " << opName << "\n";
                }
            }
        });

        LLVM_DEBUG(llvm::dbgs() << "=== Generating all Precedences ===\n");
        for (const auto &blockPtr: blocks) {
            idToBlockMap[blockPtr->getId()] = blockPtr.get();
        }

        for (const std::shared_ptr<reordering::MILPBlock> &blockPtr: blocks) {
            reordering::MILPBlock *blk = blockPtr.get();
            mlir::Block *mlirBlk = blk->getBlock();
            mlir::Operation *firstOp = &mlirBlk->front();

            qoalahost::BlkMeta blkMeta = llvm::dyn_cast<qoalahost::BlkMeta>(firstOp);
            if (!blkMeta) {
                mlir::emitError(firstOp->getLoc(), "Expected BlkMeta as first operation in block");
                return std::make_tuple(blocks, qubits, precedences, mlir::failure());
            }

            // Handle ArrayAttr-based fields (predecessors, dependencies)
            mlir::ArrayAttr predecessorsAttr = blkMeta->getAttrOfType<mlir::ArrayAttr>("predecessors");
            if (predecessorsAttr) {
                for (mlir::Attribute attrElem: predecessorsAttr) {
                    mlir::StringAttr strAttr = attrElem.dyn_cast<mlir::StringAttr>();
                    if (!strAttr)
                        continue;

                    std::string predId = strAttr.str();
                    std::unordered_map<std::string, reordering::MILPBlock *>::iterator it = idToBlockMap.find(predId);
                    if (it != idToBlockMap.end()) {
                        reordering::MILPBlock *predBlock = it->second;
                        precedences.emplace_back(predBlock, blk);
                    }
                }
            }

            mlir::ArrayAttr dependenciesAttr = blkMeta->getAttrOfType<mlir::ArrayAttr>("dependencies");
            if (dependenciesAttr) {
                for (mlir::Attribute attrElem: dependenciesAttr) {
                    mlir::StringAttr strAttr = attrElem.dyn_cast<mlir::StringAttr>();
                    if (!strAttr)
                        continue;

                    std::string predId = strAttr.str();
                    std::unordered_map<std::string, reordering::MILPBlock *>::iterator it = idToBlockMap.find(predId);
                    if (it != idToBlockMap.end()) {
                        reordering::MILPBlock *predBlock = it->second;
                        precedences.emplace_back(predBlock, blk);
                    }
                }
            }

            // Handle string-based single references (prev_ent, prev_comm)
            mlir::StringAttr prevEntAttr = blkMeta->getAttrOfType<mlir::StringAttr>("prev_ent");
            if (prevEntAttr) {
                std::string predId = prevEntAttr.str();
                std::unordered_map<std::string, reordering::MILPBlock *>::iterator it = idToBlockMap.find(predId);
                if (it != idToBlockMap.end()) {
                    reordering::MILPBlock *predBlock = it->second;
                    precedences.emplace_back(predBlock, blk);
                }
            }

            mlir::StringAttr prevCommAttr = blkMeta->getAttrOfType<mlir::StringAttr>("prev_comm");
            if (prevCommAttr) {
                std::string predId = prevCommAttr.str();
                std::unordered_map<std::string, reordering::MILPBlock *>::iterator it = idToBlockMap.find(predId);
                if (it != idToBlockMap.end()) {
                    reordering::MILPBlock *predBlock = it->second;
                    precedences.emplace_back(predBlock, blk);
                }
            }
        }

        LLVM_DEBUG({
            llvm::dbgs() << "[MILP] Precedence graph:\n";
            for (const reordering::BlockPrecedence &edge: precedences) {
                const reordering::MILPBlock *pred = edge.first;
                const reordering::MILPBlock *succ = edge.second;
                llvm::dbgs() << "  - " << pred->getId() << " → " << succ->getId() << "\n";
            }
        });

        LLVM_DEBUG(llvm::dbgs() << "=== Generating all MILPTasks ===\n");

        for (const std::shared_ptr<reordering::MILPBlock> &blockPtr: blocks) {
            reordering::MILPBlock *block = blockPtr.get();
            const std::vector<reordering::MILPOperation *> &ops = block->getOperations();

            if (ops.empty()) {
                mlir::emitError(block->getBlock()->front().getLoc())
                        << "MILPBlock '" << block->getId() << "' has no operations — this should not happen.";
                return {blocks, qubits, precedences, mlir::failure()};
            }

            // === CL or CC blocks → single task ===
            if (block->getType() == reordering::OpType::CL || block->getType() == reordering::OpType::CC) {
                if (block->getType() == reordering::OpType::CC && ops.size() != 1) {
                    mlir::emitError(block->getBlock()->front().getLoc())
                            << "MILPBlock of type CC should contain exactly one operation, but got " << ops.size();
                    return {blocks, qubits, precedences, mlir::failure()};
                }

                auto task = std::make_unique<MILPTask>("0", block, reordering::TaskGroup::C);
                for (MILPOperation *op: ops) {
                    task->addOperation(op);
                }
                block->addTask(std::move(task));
            }

            // === QL or QC blocks → split into 3 tasks ===
            else if (block->getType() == OpType::QL || block->getType() == OpType::QC) {
                if (ops.size() < 3) {
                    mlir::emitError(block->getBlock()->front().getLoc())
                            << "QL/QC block must contain at least 3 operations to form 3 tasks";
                    return {blocks, qubits, precedences, mlir::failure()};
                }

                // Task 0: first op
                // This will always be the `qoalahost.call`
                auto task0 = std::make_unique<MILPTask>("0", block, reordering::TaskGroup::C);
                task0->addOperation(ops.front());
                block->addTask(std::move(task0));

                // Task 1: middle ops
                auto task1 = std::make_unique<MILPTask>("1", block, reordering::TaskGroup::Q);
                for (size_t i = 1; i < ops.size() - 1; ++i) {
                    task1->addOperation(ops[i]);
                }
                block->addTask(std::move(task1));

                // Task 2: last op
                // This will always be the `netqasm.return`. This task is a classical task that will be executed by the
                // QNPU and not the CNPU. However, we are using it to model the PostTask. Since the time to run a
                // classical local task is negligeable compared to other tasks, this constitutes a good approximation.
                // However, it is possible in iqoala netqasm routines to have instructions to send the result to the
                // CNPU. This would come from the netqasm.return op returning something. We can maybe improve the model
                // here.
                auto task2 = std::make_unique<MILPTask>("2", block, reordering::TaskGroup::C);
                task2->addOperation(ops.back());
                block->addTask(std::move(task2));
            }
        }

        LLVM_DEBUG({
            llvm::dbgs() << "[MILP] Constructed tasks:\n";
            for (const std::shared_ptr<reordering::MILPBlock> &blockPtr: blocks) {
                const reordering::MILPBlock *block = blockPtr.get();
                for (const auto &taskPtr: block->getTasks()) {
                    const auto *task = taskPtr.get();
                    llvm::dbgs() << " - Task " << task->getId() << " (Block=" << block->getId()
                                 << ", Group=" << static_cast<int>(task->getGroup()) << ")\n";

                    for (const reordering::MILPOperation *op: task->getOperations()) {
                        llvm::dbgs() << "     * Op " << op->getId() << " ("
                                     << op->getOperation()->getName().getStringRef() << ")\n";
                    }
                }
            }
        });

        LLVM_DEBUG(llvm::dbgs() << "=== Generating all MILPQubits ===\n");

        llvm::DenseMap<mlir::Value, llvm::SmallVector<mlir::Operation *, 4>> qubitToOps;
        llvm::DenseMap<mlir::Value, mlir::Value> resolvedQubitAlias;

        auto resolve = [&](mlir::Value v) -> mlir::Value {
            llvm::SmallPtrSet<mlir::Value, 4> seen;
            while (resolvedQubitAlias.count(v)) {
                if (!seen.insert(v).second)
                    break;
                v = resolvedQubitAlias[v];
            }
            return v;
        };

        // Step 1: Traverse all call sites and track memory-effect operations
        for (auto mainFunc: moduleOp.getOps<qoalahost::MainFuncOp>()) {
            for (mlir::Block &block: mainFunc.getBody()) {
                for (mlir::Operation &op: block) {
                    auto callOp = llvm::dyn_cast<qoalahost::CallOp>(&op);
                    if (!callOp)
                        continue;

                    auto symRef = callOp.getCalleeAttr().dyn_cast<mlir::SymbolRefAttr>();
                    if (!symRef)
                        continue;

                    auto *callee = mlir::SymbolTable::lookupNearestSymbolFrom(moduleOp, symRef);
                    if (!callee || callee->getNumRegions() == 0)
                        continue;

                    mlir::Block &entry = callee->getRegion(0).front();
                    mlir::ValueRange actualArgs = callOp->getOperands();
                    auto formalArgs = entry.getArguments();

                    llvm::DenseMap<mlir::Value, mlir::Value> argMap;
                    for (size_t i = 0; i < std::min(formalArgs.size(), actualArgs.size()); ++i)
                        argMap[formalArgs[i]] = actualArgs[i];

                    // Gather memory effects inside callee
                    for (auto &innerOp: entry) {
                        if (auto memOp = llvm::dyn_cast<mlir::MemoryEffectOpInterface>(&innerOp)) {
                            llvm::SmallVector<mlir::MemoryEffects::EffectInstance, 4> effects;
                            memOp.getEffects(effects);
                            for (auto &eff: effects) {
                                mlir::Value val = eff.getValue();
                                if (!val)
                                    continue;

                                mlir::Value resolved = argMap.lookup(val);
                                if (!resolved)
                                    resolved = val;

                                // Do not resolve yet — we merge later
                                mlir::Value canonical = resolve(resolved);
                                qubitToOps[canonical].push_back(&innerOp);
                                LLVM_DEBUG(llvm::dbgs()
                                           << "    [Track] " << innerOp.getName() << " → " << canonical << "\n");
                            }
                        }
                    }

                    // Track result → returned value alias
                    if (callOp->getNumResults() > 0) {

                        for (auto &block: callee->getRegion(0)) {
                            for (auto &innerOp: block) {
                                if (llvm::isa<netqasm::ReturnOp>(innerOp)) {

                                    auto retVals = innerOp.getOperands();
                                    auto callResults = callOp->getResults();

                                    for (size_t i = 0; i < std::min(retVals.size(), callResults.size()); ++i) {
                                        mlir::Value res = callResults[i];
                                        mlir::Value retVal = retVals[i];
                                        mlir::Value final = resolve(retVal);

                                        auto *defOp = final.getDefiningOp();
                                        if (defOp && isa<netqasm::QAllocOp>(defOp)) {
                                            resolvedQubitAlias[res] = final;
                                            LLVM_DEBUG(llvm::dbgs() << "  [Alias→QAlloc] " << res << " ↦ " << final
                                                                    << " (qalloc)\n");
                                        } else {
                                            LLVM_DEBUG(llvm::dbgs() << "  [Skip Alias] " << res << " ↦ " << final
                                                                    << " (not qalloc)\n");
                                        }
                                    }

                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

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

        for (auto &entry: qubitToOps) {
            mlir::Value qubit = entry.first;
            const auto &ops = entry.second;

            std::string id = "q" + std::to_string(qubitIndex++);
            auto qubitPtr = std::make_shared<MILPQubit>(id);

            MILPOperation *allocOp = nullptr;
            MILPOperation *measOp = nullptr;

            for (mlir::Operation *op: ops) {
                llvm::StringRef name = op->getName().getStringRef();

                if (llvm::isa<netqasm::QInitOp>(op) || llvm::isa<netqasm::EprsOp>(op)) {
                    allocOp = opToMilpOp.count(op) ? opToMilpOp[op] : nullptr;
                }

                if (llvm::isa<netqasm::MeasureOp>(op)) {
                    measOp = opToMilpOp.count(op) ? opToMilpOp[op] : nullptr;
                }
            }

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

        return {blocks, qubits, precedences, mlir::success()};
    }

    MILPModelBuilder::MILPModelBuilder() : scip_(nullptr) {}

    MILPModelBuilder::~MILPModelBuilder() { cleanup(); }

    bool MILPModelBuilder::initialize() {
        SCIP_RETCODE retcode = SCIPcreate(&scip_);
        if (retcode != SCIP_OKAY)
            return false;

        retcode = SCIPincludeDefaultPlugins(scip_);
        return retcode == SCIP_OKAY;
    }

    void MILPModelBuilder::setProblemData(const std::vector<std::shared_ptr<MILPBlock>> &blocks,
                                          const std::vector<std::shared_ptr<MILPQubit>> &qubits) {
        blocks_ = blocks;
        qubits_ = qubits;
    }

    void MILPModelBuilder::createVariables() {
        // Placeholder: create SCIP_VARs and store in startVars_
    }

    void MILPModelBuilder::addConstraints() {
        // Placeholder: call individual constraint-adding methods
        addIntraTaskOrderingConstraints();
        addBlockPrecedenceConstraints();
        addFCFSTaskConstraints();
        addIntraBlockSequencingConstraints();
    }

    void MILPModelBuilder::addIntraTaskOrderingConstraints() {
        // Placeholder
    }

    void MILPModelBuilder::addBlockPrecedenceConstraints() {
        // Placeholder
    }

    void MILPModelBuilder::addFCFSTaskConstraints() {
        // Placeholder
    }

    void MILPModelBuilder::addIntraBlockSequencingConstraints() {
        // Placeholder
    }

    void MILPModelBuilder::setObjective() {
        // Placeholder
    }

    bool MILPModelBuilder::optimize() {
        SCIP_RETCODE retcode = SCIPsolve(scip_);
        return retcode == SCIP_OKAY;
    }

    double MILPModelBuilder::getOperationStartTime(const std::string &opId) const {
        auto it = startVars_.find(opId);
        if (it == startVars_.end())
            return -1.0;
        return SCIPgetSolVal(scip_, nullptr, it->second);
    }

    void MILPModelBuilder::cleanup() {
        if (scip_ != nullptr) {
            SCIPfree(&scip_);
            scip_ = nullptr;
        }
    }

    SCIP_VAR *MILPModelBuilder::createContinuousVariable(const std::string &name, double lb, double ub) {
        // Placeholder: actual SCIPvarCreateBasic call should go here later
        return nullptr;
    }
} // namespace qoala::analysis::reordering
