#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "llvm/Support/Debug.h"
#include "mlir/IR/SymbolTable.h"
#include "Dialect/QoalaHost/Passes.h"
#include "llvm/Support/FormatVariadic.h"
#include "Tools/QoalaOpt.h"
#include <set>
#include <unordered_set>

#include "scip/scip.h"
#include "scip/scipdefplugins.h"
#include <scip/cons.h>
#include <scip/cons_linear.h>
#include <scip/expr_sum.h>

#define DEBUG_TYPE "qoalahost-reorder-blocks-pass-internal"

using namespace mlir;
using namespace qoala::dialects;
using namespace qoala::analysis;

namespace qoala::analysis::reordering {
    double getOperationDuration(mlir::Operation *op) {
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

    OpType inferTypeFromCall(mlir::Operation *op, mlir::ModuleOp moduleOp) {
        if (auto callOp = llvm::dyn_cast<qoalahost::CallOp>(op)) {
            auto symRef = callOp.getCalleeAttr().dyn_cast_or_null<mlir::SymbolRefAttr>();
            if (!symRef)
                return OpType::CL;
            auto callee = mlir::SymbolTable::lookupNearestSymbolFrom(moduleOp, symRef);
            if (llvm::isa<netqasm::RequestRoutineOp>(callee))
                return OpType::QC;
            if (llvm::isa<netqasm::LocalRoutineOp>(callee))
                return OpType::QL;
        }
        return OpType::CL;
    }

    mlir::LogicalResult createTasksForBlock(MILPBlock *blk, const mlir::Location &loc) {
        const auto &ops = blk->getOperations();
        if (ops.empty()) {
            mlir::emitError(loc) << "MILPBlock '" << blk->getId() << "' has no operations — this should not happen.";
            return mlir::failure();
        }

        using TG = TaskGroup;

        switch (blk->getType()) {
            case OpType::CL:
            case OpType::CC: {
                if (blk->getType() == OpType::CC && ops.size() != 1) {
                    mlir::emitError(loc) << "MILPBlock of type CC should contain exactly one operation, but got "
                                         << ops.size();
                    return mlir::failure();
                }
                auto task = std::make_unique<MILPTask>("0", blk, TG::C);
                for (auto *op: ops)
                    task->addOperation(op);
                blk->addTask(std::move(task));
                break;
            }
            case OpType::QL:
            case OpType::QC: {
                if (ops.size() < 3) {
                    mlir::emitError(loc) << "QL/QC block must contain at least 3 operations to form 3 tasks";
                    return mlir::failure();
                }
                // Task 0 – call (C): PreTask
                auto t0 = std::make_unique<MILPTask>("0", blk, TG::C);
                t0->addOperation(ops.front());
                blk->addTask(std::move(t0));
                // Task 1 – middle (Q): Quantum Routine
                auto t1 = std::make_unique<MILPTask>("1", blk, TG::Q);
                for (size_t i = 1; i + 1 < ops.size(); ++i)
                    t1->addOperation(ops[i]);
                blk->addTask(std::move(t1));
                // Task 2 – return (C) : PostTask.
                auto t2 = std::make_unique<MILPTask>("2", blk, TG::C);
                t2->addOperation(ops.back());
                blk->addTask(std::move(t2));
                break;
            }
        }
        return mlir::success();
    }

    std::tuple<std::vector<std::shared_ptr<MILPBlock>>, std::vector<std::shared_ptr<MILPQubit>>, BlockPrecedenceList,
               mlir::LogicalResult>
    buildMILPFromMLIR(mlir::ModuleOp moduleOp) {
        std::vector<std::shared_ptr<MILPBlock>> blocks;
        std::vector<std::shared_ptr<MILPQubit>> qubits;
        BlockPrecedenceList precedences;

        llvm::StringMap<MILPBlock *> idToBlockMap;
        std::unordered_map<mlir::Operation *, MILPOperation *> opToMilpOp;
        std::vector<std::pair<std::string, std::string>> unresolvedEdges; // (predId,succId)

        auto mainFuncs = moduleOp.getOps<qoalahost::MainFuncOp>();
        if (mainFuncs.empty()) {
            mlir::emitError(moduleOp.getLoc(), "No main function found in module");
            return {blocks, qubits, precedences, mlir::failure()};
        }
        qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();

        llvm::SmallPtrSet<mlir::Block *, 32> visitedBlocks;
        mlir::LogicalResult status = mlir::success();

        // Walk every BlkMeta once and build blocks / tasks / edges
        mainFunc.walk([&](qoalahost::BlkMeta blkMeta) {
            if (mlir::failed(status))
                return;

            mlir::Block *block = blkMeta->getBlock();
            if (!visitedBlocks.insert(block).second)
                return;

            // Determine block type from first non‑meta op
            auto firstIt = std::next(block->begin());
            if (firstIt == block->end()) {
                mlir::emitError(blkMeta.getLoc(), "Block has no body after BlkMeta");
                status = mlir::failure();
                return;
            }
            mlir::Operation *firstOp = &*firstIt;

            OpType blkType = OpType::CL;
            if (llvm::isa<qoalahost::SendIntsOp>(firstOp) || llvm::isa<qoalahost::RecvIntsOp>(firstOp) ||
                llvm::isa<qoalahost::SendFloatsOp>(firstOp) || llvm::isa<qoalahost::RecvFloatsOp>(firstOp)) {
                blkType = OpType::CC;
            } else {
                blkType = inferTypeFromCall(firstOp, moduleOp);
            }

            std::string blkId = blkMeta.getBlockId().str();
            auto blkPtr = std::make_shared<MILPBlock>(blkId, blkType);
            blkPtr->setBlock(block);
            MILPBlock *blk = blkPtr.get();
            idToBlockMap[blkId] = blk;

            int opIdx = 0;

            for (auto it = firstIt; it != block->end(); ++it) {
                mlir::Operation *op = &*it;

                if (blkType == OpType::CL && (llvm::isa<qoalahost::ReturnOp>(op) || llvm::isa<qoalahost::NopTOp>(op)))
                    break; // stop at terminator‑like op in CL block

                // Create MILPOperation
                std::string opId = blkId + "::" + std::to_string(opIdx++);
                auto *milpOp = new MILPOperation(opId, blkType, getOperationDuration(op));
                milpOp->setOperation(op);
                blk->addOperation(milpOp);
                opToMilpOp[op] = milpOp;

                // Inline body for QL/QC blocks immediately after call
                if (blkType == OpType::QL || blkType == OpType::QC) {
                    if (auto callOp = llvm::dyn_cast<qoalahost::CallOp>(op)) {
                        auto sym = callOp.getCalleeAttr().dyn_cast_or_null<mlir::SymbolRefAttr>();
                        auto callee = sym ? mlir::SymbolTable::lookupNearestSymbolFrom(moduleOp, sym) : nullptr;
                        auto calleeFunc = llvm::dyn_cast_or_null<mlir::FunctionOpInterface>(callee);
                        if (!calleeFunc) {
                            mlir::emitError(op->getLoc(), "Callee is not a FunctionOpInterface");
                            status = mlir::failure();
                            return;
                        }
                        bool foundReturn = false;
                        for (mlir::Block &cb: calleeFunc->getRegion(0)) {
                            for (mlir::Operation &cOp: cb) {
                                std::string subId = blkId + "::" + std::to_string(opIdx++);
                                auto *milpSub = new MILPOperation(subId, blkType, getOperationDuration(&cOp));
                                milpSub->setOperation(&cOp);
                                blk->addOperation(milpSub);
                                opToMilpOp[&cOp] = milpSub;
                                if (llvm::isa<netqasm::ReturnOp>(cOp)) {
                                    // Insert qoalahost.NopOp in caller block & track to model PostTask
                                    mlir::OpBuilder b(moduleOp.getContext());
                                    b.setInsertionPointToEnd(block);
                                    auto nop = b.create<qoalahost::NopOp>(cOp.getLoc());

                                    std::string nopId = blkId + "::" + std::to_string(opIdx++);
                                    auto *milpNop =
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
                            mlir::emitError(op->getLoc(), "Callee does not end in netqasm::ReturnOp");
                            status = mlir::failure();
                        }
                        break; // after inlined body
                    }
                }

                if (blkType == OpType::CC)
                    break; // single‑op block for CC
            }

            if (mlir::failed(status))
                return;

            // Precedence edges: for our optimization all types of precedences are equivalent.
            // The differenciation is needed for the translation step.
            auto recordEdge = [&](mlir::StringRef predId) {
                if (predId.empty())
                    return;
                auto it = idToBlockMap.find(predId);
                if (it != idToBlockMap.end())
                    precedences.emplace_back(it->second, blk);
                else
                    unresolvedEdges.emplace_back(predId.str(), blkId);
            };
            if (auto a = blkMeta.getPredecessorsAttr())
                for (auto s: a.getAsValueRange<mlir::StringAttr>())
                    recordEdge(s);
            if (auto a = blkMeta.getDependenciesAttr())
                for (auto s: a.getAsValueRange<mlir::StringAttr>())
                    recordEdge(s);
            if (auto a = blkMeta.getPrevEntAttr())
                recordEdge(a.getValue());
            if (auto a = blkMeta.getPrevCommAttr())
                recordEdge(a.getValue());

            // Tasks
            if (mlir::failed(createTasksForBlock(blk, blkMeta.getLoc()))) {
                status = mlir::failure();
                return;
            }

            blocks.push_back(std::move(blkPtr));
        });

        if (mlir::failed(status))
            return {blocks, qubits, precedences, mlir::failure()};

        // Fail if unresolved edges remain
        if (!unresolvedEdges.empty()) {
            mlir::emitError(moduleOp.getLoc(), "Unresolved precedence edges detected");
            return {blocks, qubits, precedences, mlir::failure()};
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

        // Traverse all call sites and track memory-effect operations
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
        for (const auto &blk: blocks_)
            for (const auto *op: blk->getOperations()) {
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


    void MILPModelBuilder::addConstraints() {
        // Call individual constraint-adding methods
        addIntraTaskOrderingConstraints();
        addBlockPrecedenceConstraints();
        addFCFSTaskConstraints();
        addIntraBlockSequencingConstraints();
    }

    // equality chain inside each task
    void MILPModelBuilder::addIntraTaskOrderingConstraints() {
        for (const auto &blk: blocks_) {
            for (const auto &t: blk->getTasks()) {
                const auto &ops = t->getOperations();
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
            const auto *pred = e.first;
            const auto *succ = e.second;
            const auto *predLast = pred->lastOp();
            const auto *succFirst = succ->firstOp();

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

    using Closure = std::set<std::pair<std::string, std::string>>;
    static bool reachable(const MILPBlock *a, const MILPBlock *b, const Closure &C) {
        return C.count({a->getId(), b->getId()}) > 0;
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
            for (auto &p: clos)
                for (auto &q: clos)
                    if (p.second == q.first && clos.insert({p.first, q.second}).second)
                        grown = true;
        } while (grown);

        const int B = static_cast<int>(blocks_.size());
        for (int i = 0; i < B; ++i) {
            const auto *b1 = blocks_[i].get();
            for (int j = i + 1; j < B; ++j) {
                const auto *b2 = blocks_[j].get();

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

                const auto &t1 = b1->getTasks();
                const auto &t2 = b2->getTasks();
                const std::vector<int> idx =
                        (t1.size() == 3 && t2.size() == 3) ? std::vector<int>{0, 1, 2} : std::vector<int>{0};
                for (int k: idx) {
                    const auto *o1 = t1[k]->getOperations().front();
                    const auto *o2 = t2[k]->getOperations().front();
                    double dur1 = 0.0, dur2 = 0.0;
                    for (auto *op: t1[k]->getOperations())
                        dur1 += op->getDuration();
                    for (auto *op: t2[k]->getOperations())
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
        for (const auto &blk: blocks_) {
            if (blk->getType() != OpType::QL && blk->getType() != OpType::QC)
                continue;
            const auto &tasks = blk->getTasks();
            if (tasks.size() != 3)
                continue;

            const auto *t1 = tasks[0].get();
            const auto *t2 = tasks[1].get();
            const auto *t3 = tasks[2].get();

            const auto *first2 = t2->getOperations().front();
            const auto *first3 = t3->getOperations().front();

            double dur1 = 0.0, dur2 = 0.0;
            for (auto *op: t1->getOperations())
                dur1 += op->getDuration();
            for (auto *op: t2->getOperations())
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

        for (const auto &q: qubits_) {
            const auto *alloc = q->getAllocation();
            const auto *meas = q->getMeasurement();
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
        for (auto &[_, var]: startVars_)
            SCIPreleaseVar(scip_, &var);
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
} // namespace qoala::analysis::reordering
