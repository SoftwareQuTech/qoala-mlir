#include "Analysis/QoalaHost/AnalysisTraversal.h"
#include "Analysis/QoalaHost/Helpers.h"
#include "Analysis/QoalaHost/QubitLife.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/IR/AsmState.h"
#include "mlir/IR/Operation.h"

#define DEBUG_TYPE "qoalahost-qubit-life"

using namespace mlir;
using namespace qoala::dialects;
using namespace qoala::analysis;

namespace qoala::analysis::qubitlife {

    // Encapsulates the mutable task accumulation tasks during traversal.
    struct BranchTasks {
        std::vector<Task> cpuTasks;
        std::vector<Task> qpuTasks;
        std::unordered_map<std::string, std::vector<std::string>> taskDependences;
    };

    // Result of running the scheduler on a complete path's accumulated tasks.
    struct SchedulerResult {
        std::unordered_map<std::string, uint64_t> qubitLifetimes;
        uint64_t globalTime;
    };

    /**
     * Construct LiveQubit objects
     * - Extract alloc, meas and last two-qubit op from qubit usage
     * - Create LiveQubit and attach relevant operations
     * TODO This could be merged with the check for MeasureOp,
     * removing the need of the LiveQubit class.
     * Need to check if MILP reordering would still work even when
     * the measure op is instead a two-qubit op. See issue qoala-kanban-board#109.
     */
    static std::vector<std::shared_ptr<LiveQubit>>
    getQubitCriticalOps(const llvm::DenseMap<Value, std::vector<Operation *>> &qubitToOps,
                        const std::unordered_map<Operation *, reordering::MILPOperation *> &opToMilpOp) {
        std::vector<std::shared_ptr<LiveQubit>> qubits;

        for (const auto &entry : qubitToOps) {
            const std::vector<Operation *> &ops = entry.second;

            reordering::MILPOperation *allocOp = nullptr;
            reordering::MILPOperation *measOp = nullptr;
            reordering::MILPOperation *twoQubitOp = nullptr;

            for (Operation *op : ops) {
                if (llvm::isa<netqasm::QInitOp>(op) || llvm::isa<netqasm::EprsOp>(op)) {
                    auto it = opToMilpOp.find(op);
                    allocOp = (it != opToMilpOp.end()) ? it->second : nullptr;
                }

                if (llvm::isa<netqasm::MeasureOp>(op)) {
                    auto itMeas = opToMilpOp.find(op);
                    measOp = (itMeas != opToMilpOp.end()) ? itMeas->second : nullptr;
                }

                // Treat last two-qubit op as a measure op.
                // A measure op will overwrite the last two-qubit op.
                if (llvm::isa<netqasm::ifaces::DualQubitOp>(op)) {
                    auto itTwoQubitOp = opToMilpOp.find(op);
                    twoQubitOp = (itTwoQubitOp != opToMilpOp.end()) ? itTwoQubitOp->second : nullptr;
                }
            }

            assert(allocOp != nullptr && "Missing Alloc Op for qubit.");

            std::string id = allocOp->getId();
            auto qubitPtr = std::make_shared<LiveQubit>(id);

            // Attach known alloc/meas to the qubit model object
            qubitPtr->setAllocation(allocOp);
            if (measOp) {
                qubitPtr->setMeasurement(measOp);
            }
            if (twoQubitOp) {
                qubitPtr->setTwoQubitOp(twoQubitOp);
            }

            qubits.push_back(std::move(qubitPtr));
        }

        return qubits;
    }

    static void buildQubitMaps(const std::vector<std::shared_ptr<LiveQubit>> &qubits,
                               std::unordered_map<std::string, std::string> &qubitInits,
                               std::unordered_map<std::string, std::string> &qubitMeas) {

        qubitInits.reserve(qubits.size());
        qubitMeas.reserve(qubits.size());

        for (const auto &qubit : qubits) {
            const std::string &qubitId = qubit->getId();
            assert(qubit->getAllocation() != nullptr && "Alloc Op not found for qubit.");
            const std::string &allocId = qubit->getAllocation()->getId();
            LLVM_DEBUG(llvm::dbgs() << "Qubit '" << qubitId << "' initialized in '" << allocId << "'.\n");
            auto measPtr = qubit->getMeasurement();
            if (measPtr == nullptr) {
                measPtr = qubit->getTwoQubitOp();
            }

            if (measPtr != nullptr) {
                // If not measured or no two-qubit op, qubit is not tracked at all
                qubitInits.emplace(allocId, qubitId);
                const std::string &measId = measPtr->getId();
                qubitMeas.emplace(measId, qubitId);
                LLVM_DEBUG(llvm::dbgs() << "Qubit '" << qubitId << "' initialized in '" << allocId
                                        << "' and measured in '" << measId << "'.\n");
            }
        }
    }

    /**
     * Convert BlockPrerequisites (Block* -> Block*) to MILPBlock-level dependencies
     * (string -> MILPBlock*), merging both predecessors and dependencies into a single map
     * since processBlock treats all dependency types uniformly for inter-block task ordering.
     */
    static std::unordered_map<std::string, std::vector<reordering::MILPBlock *>>
    convertPrereqsToMilpDeps(const BlockPrerequisites &prereqs,
                             const llvm::DenseMap<mlir::Block *, reordering::MILPBlock *> &blockToMilpBlock) {

        std::unordered_map<std::string, std::vector<reordering::MILPBlock *>> blockDependences;

        auto addDeps = [&](const mlir::DenseMap<mlir::Block *, llvm::SmallVector<mlir::Block *, 2>> &map) {
            for (const auto &[block, deps] : map) {
                auto milpIt = blockToMilpBlock.find(block);
                if (milpIt == blockToMilpBlock.end()) {
                    continue;
                }
                const std::string &blkId = milpIt->second->getId();
                for (mlir::Block *dep : deps) {
                    auto depMilpIt = blockToMilpBlock.find(dep);
                    if (depMilpIt != blockToMilpBlock.end()) {
                        blockDependences[blkId].push_back(depMilpIt->second);
                    }
                }
            }
        };

        addDeps(prereqs.dependencies);
        addDeps(prereqs.predecessors);

        return blockDependences;
    }

    /**
     * Process a block and separate qpu and cpu tasks.
     * Track dependences between tasks.
     * Register qubit initialization and measurement tasks.
     */
    static std::tuple<std::string, std::string>
    processBlock(const reordering::MILPBlock *block,
                 const std::unordered_map<std::string, std::vector<reordering::MILPBlock *>> &blockDependences,
                 std::unordered_map<std::string, std::vector<std::string>> &taskDependences,
                 std::vector<Task> &qpuTasks, std::vector<Task> &cpuTasks,
                 const std::unordered_map<std::string, std::string> &qubitInits,
                 const std::unordered_map<std::string, std::string> &qubitMeas) {

        std::string intraBlockPred;
        std::string last;
        uint32_t numTasksAddedForBlock = 0;

        const auto depIt = blockDependences.find(block->getId());
        const bool interBlockPred = (depIt != blockDependences.end());

        LLVM_DEBUG(llvm::dbgs() << "Block '" << block->getId() << "' has inter block dependency: " << interBlockPred
                                << "\n");

        for (const auto &tPtr : block->getTasks()) {
            const auto *t = tPtr.get();

            uint64_t taskTime = 0;

            for (const auto *op : t->getOperations()) {
                taskTime += op->getDuration();

                const std::string &opId = op->getId();
                auto initIt = qubitInits.find(opId);
                auto measIt = qubitMeas.find(opId);

                if (initIt != qubitInits.end() || measIt != qubitMeas.end()) {
                    qpuTasks.emplace_back(opId, taskTime);
                    taskDependences.try_emplace({opId, {}});
                    numTasksAddedForBlock++;

                    LLVM_DEBUG(llvm::dbgs()
                               << "Added Task '" << opId << "' with execution time: " << taskTime << ".\n");

                    taskTime = 0;

                    if (!intraBlockPred.empty()) {
                        LLVM_DEBUG(llvm::dbgs()
                                   << "Task has intra block dependency with '" << intraBlockPred << "'.\n");
                        taskDependences.at(opId).push_back(intraBlockPred);
                    }
                    intraBlockPred = opId;
                }
                last = op->getId();
            }
            if (taskTime != 0) {
                if (t->getGroup() == analysis::reordering::TaskGroup::Q) {
                    qpuTasks.emplace_back(last, taskTime);
                } else {
                    cpuTasks.emplace_back(last, taskTime);
                }
                taskDependences.insert({last, {}});
                numTasksAddedForBlock++;
                LLVM_DEBUG(llvm::dbgs() << "Added Task '" << last << "' with execution time: " << taskTime << ".\n");
                taskTime = 0;
                if (!intraBlockPred.empty()) {
                    LLVM_DEBUG(llvm::dbgs() << "Task has intra block dependency with '" << intraBlockPred << "'.\n");
                    taskDependences.at(last).push_back(intraBlockPred);
                }
                intraBlockPred = last;
            }
        }

        // If no tasks were added to this block, create a placeholder task
        if (numTasksAddedForBlock == 0) {
            std::string placeholderTask = block->getId();
            cpuTasks.emplace_back(placeholderTask, 0);
            taskDependences.try_emplace({placeholderTask, {}});
            last = placeholderTask;
            numTasksAddedForBlock = 1;
            LLVM_DEBUG(llvm::dbgs() << "Created placeholder task '" << placeholderTask << "' for empty block.\n");
        }

        if (interBlockPred == true) {
            for (const auto &temp : depIt->second) {
                std::string lastBlockTask;
                if (temp->getTasks().empty()) {
                    // If predecessor block has no tasks, use its ID as the task name
                    lastBlockTask = temp->getId();
                } else {
                    lastBlockTask = temp->getTasks().back()->getOperations().back()->getId();
                }
                // With purely classicla blocks, all ops will be grouped with the last task, use it for inter block pred
                auto currentBlockTask = last;
                // With quantum blocks, ops may be divided into multiple groups of tasks, use the first (i.e. the
                // pre-task) for inter block dep
                if (numTasksAddedForBlock > 1) {
                    currentBlockTask = block->getTasks().front()->getOperations().front()->getId();
                }
                LLVM_DEBUG(llvm::dbgs() << "Task " << currentBlockTask << " has inter block dependency with '"
                                        << lastBlockTask << "'.\n");
                taskDependences.at(currentBlockTask).push_back(lastBlockTask);
            }
        }

        std::string lastTask = last;
        std::string firstTask = last;
        if (numTasksAddedForBlock > 1) {
            firstTask = block->getTasks().front()->getOperations().front()->getId();
        }
        return {firstTask, lastTask};
    }

    /**
     * Verify if a task is available for scheduling.
     * A task is available if all its dependences have already been scheduled.
     */
    static bool isTaskAvailable(const std::string &taskName,
                                const std::unordered_map<std::string, std::vector<std::string>> &taskDependences) {
        const auto it = taskDependences.find(taskName);
        // Maybe this check is not needed
        if (it == taskDependences.end()) {
            return false;
        }

        const auto &dependencies = it->second;
        return std::all_of(dependencies.begin(), dependencies.end(), [&taskDependences](const std::string &dep) {
            return taskDependences.find(dep) == taskDependences.end();
        });
    }

    /**
     * Find next task available to be scheduled in a list of tasks.
     * Return the index of the available task, or std::nullopt if no task is available.
     */
    static std::optional<size_t>
    findNextAvailableTask(const std::vector<Task> &tasks,
                          const std::unordered_map<std::string, std::vector<std::string>> &taskDependences) {

        for (size_t i = 0; i < tasks.size(); ++i) {
            if (isTaskAvailable(tasks[i].getName(), taskDependences)) {
                return i;
            }
        }
        return std::nullopt;
    }

    /**
     * Update qubit life times based on scheduled task type.
     * - If task is an init task, set the start time
     * - If task is a measurement task, compute the life time
     * - Do nothing otherwise
     */
    static void updateQubitLifetime(const Task &scheduledTask, uint64_t currentTime,
                                    const std::unordered_map<std::string, std::string> &qubitInits,
                                    const std::unordered_map<std::string, std::string> &qubitMeas,
                                    std::unordered_map<std::string, uint64_t> &qubitLifetimes) {

        // Check if it is an init task
        auto initIt = qubitInits.find(scheduledTask.getName());
        if (initIt != qubitInits.end()) {
            // Life time should start after initialization is completed
            qubitLifetimes.emplace(initIt->second, currentTime);
        }

        // Check if it is a measurement task
        auto measIt = qubitMeas.find(scheduledTask.getName());
        if (measIt != qubitMeas.end()) {
            auto lifetimeIt = qubitLifetimes.find(measIt->second);
            if (lifetimeIt != qubitLifetimes.end()) {
                const uint64_t initTime = lifetimeIt->second;
                lifetimeIt->second = currentTime - initTime;
                LLVM_DEBUG(llvm::dbgs() << "Qubit '" << measIt->second << "' measured at time " << currentTime
                                        << ", lifetime: " << lifetimeIt->second << "\n");
            }
        }
    }

    /**
     * Schedule a task if ready, removing it form the available tasks, and update related data.
     * Return true if the task has been scheduled.
     */
    static bool scheduleTaskIfReady(std::vector<Task> &tasks, const std::optional<size_t> taskIndex, uint64_t &taskTime,
                                    const uint64_t globalTime,
                                    std::unordered_map<std::string, std::vector<std::string>> &taskDependences,
                                    const std::unordered_map<std::string, std::string> &qubitInits,
                                    const std::unordered_map<std::string, std::string> &qubitMeas,
                                    std::unordered_map<std::string, uint64_t> &qubitLifetimes) {

        if (!taskIndex.has_value()) {
            taskTime = globalTime;
            return false;
        }

        const Task &task = tasks[*taskIndex];

        // A task is ready and can be scheduled only if
        // its execution time would not increase the time
        // of its task type beyond the global time.
        if (globalTime - taskTime < task.getTime()) {
            return false;
        }

        LLVM_DEBUG(llvm::dbgs() << "Scheduling Task: " << task.getName() << " at time " << globalTime << "\n");

        // Update qubit lifetime if needed
        updateQubitLifetime(task, globalTime, qubitInits, qubitMeas, qubitLifetimes);

        // Update the tasks
        taskTime = globalTime;

        // Remove the task
        taskDependences.erase(task.getName());
        tasks.erase(tasks.begin() + *taskIndex);

        return true;
    }

    /**
     * Compute the next global time increment.
     */
    static uint64_t computeNextTimeIncrement(const std::vector<Task> &cpuTasks, const std::vector<Task> &qpuTasks,
                                             const std::optional<size_t> nextCpuTaskIdx,
                                             const std::optional<size_t> nextQpuTaskIdx, const uint64_t cpuTime,
                                             const uint64_t qpuTime, const uint64_t currentTime) {
        const bool hasCpuTask = nextCpuTaskIdx.has_value() && *nextCpuTaskIdx < cpuTasks.size();
        const bool hasQpuTask = nextQpuTaskIdx.has_value() && *nextQpuTaskIdx < qpuTasks.size();

        if (!hasCpuTask && !hasQpuTask) {
            return 0;
        }

        if (!hasCpuTask) {
            return qpuTasks[*nextQpuTaskIdx].getTime() - (currentTime - qpuTime);
        }

        if (!hasQpuTask) {
            return cpuTasks[*nextCpuTaskIdx].getTime() - (currentTime - cpuTime);
        }

        // If both cpu and qpu tasks are found, select the one with the shortes execution time
        // This enables to keep track of parallel cpu and qpu tasks execution,
        // where first the shorter cpu tasks are scheduled, up until no other cpu tasks are avialable
        // or the global time is enough to fit in the qpu tasks (now scheduled in parallel with all
        // the already scheduled cpu tasks).
        const uint64_t cpuIncrement = cpuTasks[*nextCpuTaskIdx].getTime() - (currentTime - cpuTime);
        const uint64_t qpuIncrement = qpuTasks[*nextQpuTaskIdx].getTime() - (currentTime - qpuTime);
        return std::min(cpuIncrement, qpuIncrement);
    }

    /**
     * Run the scheduler to completion on a set of accumulated tasks along an exection path.
     * Simulates parallel CPU/QPU execution and computes qubit lifetimes.
     */
    static SchedulerResult runScheduler(BranchTasks tasks,
                                        const std::unordered_map<std::string, std::string> &qubitInits,
                                        const std::unordered_map<std::string, std::string> &qubitMeas) {

        uint64_t cpuTime = 0;
        uint64_t qpuTime = 0;
        uint64_t globalTime = 0;
        std::unordered_map<std::string, uint64_t> qubitLifetimes;

        LLVM_DEBUG(llvm::dbgs() << "\n=== Starting Task Scheduling ===\n");

        while (!tasks.cpuTasks.empty() || !tasks.qpuTasks.empty()) {
            auto nextQpuTaskIdx = findNextAvailableTask(tasks.qpuTasks, tasks.taskDependences);
            auto nextCpuTaskIdx = findNextAvailableTask(tasks.cpuTasks, tasks.taskDependences);

            uint64_t timeIncrement = computeNextTimeIncrement(tasks.cpuTasks, tasks.qpuTasks, nextCpuTaskIdx,
                                                              nextQpuTaskIdx, cpuTime, qpuTime, globalTime);

            globalTime += timeIncrement;
            LLVM_DEBUG(llvm::dbgs() << "Global Time: " << globalTime << "\n");

            scheduleTaskIfReady(tasks.qpuTasks, nextQpuTaskIdx, qpuTime, globalTime, tasks.taskDependences, qubitInits,
                                qubitMeas, qubitLifetimes);
            scheduleTaskIfReady(tasks.cpuTasks, nextCpuTaskIdx, cpuTime, globalTime, tasks.taskDependences, qubitInits,
                                qubitMeas, qubitLifetimes);
        }

        LLVM_DEBUG(llvm::dbgs() << "\n=== Qubit Lifetimes ===\n");
        for (const auto &[qubitId, lifetime] : qubitLifetimes) {
            LLVM_DEBUG(llvm::dbgs() << qubitId << ": " << lifetime << "\n");
        }
        LLVM_DEBUG(llvm::dbgs() << "\nTotal Execution Time: " << globalTime << "\n");

        return SchedulerResult{qubitLifetimes, globalTime};
    }

    /**
     * Traverse blocks using blk_meta prerequisites and accumulate tasks for scheduling.
     * Uses scanForReadyBlock to find the next block whose blk_meta prerequisites are met.
     * At conditional branches, forks recursively with a forbidden set (condBrTargets)
     * to prevent cross-branch contamination.
     */
    static SchedulerResult traverseCFGAndSchedule(
            mlir::Block *startBlock, llvm::DenseSet<mlir::Block *> visited, BranchTasks tasks,
            const llvm::DenseMap<mlir::Block *, reordering::MILPBlock *> &blockToMilpBlock,
            const std::unordered_map<std::string, std::vector<reordering::MILPBlock *>> &blockDependences,
            const std::unordered_map<std::string, std::string> &qubitInits,
            const std::unordered_map<std::string, std::string> &qubitMeas, const BlockPrerequisites &prereqs,
            llvm::DenseSet<mlir::Block *> condBrTargets) {

        Block *block = startBlock;
        // track last task for cf.br ordering
        std::string prevBlockLastTask;
        while (block) {
            if (visited.contains(block)) {
                return runScheduler(std::move(tasks), qubitInits, qubitMeas);
            }
            visited.insert(block);

            // Process this block if it has a corresponding MILPBlock
            auto milpIt = blockToMilpBlock.find(block);
            if (milpIt != blockToMilpBlock.end()) {
                auto [firstTask, lastTask] = processBlock(milpIt->second, blockDependences, tasks.taskDependences,
                                                          tasks.qpuTasks, tasks.cpuTasks, qubitInits, qubitMeas);

                // If this block was reached via cf.br, add ordering dependency
                if (!prevBlockLastTask.empty() && !firstTask.empty()) {
                    tasks.taskDependences.at(firstTask).push_back(prevBlockLastTask);
                    LLVM_DEBUG(llvm::dbgs()
                               << "cf.br ordering: " << firstTask << " depends on " << prevBlockLastTask << "\n");
                }
                prevBlockLastTask = lastTask;
            }

            // Detect branch operations (cf ops used only for branch detection;
            // cannot use getTerminator() because buildMilpBlocks inserts synthetic
            // qoalahost.nop operations after call terminators).
            cf::CondBranchOp condBr = nullptr;
            cf::BranchOp br = nullptr;
            for (auto &op : block->getOperations()) {
                if (auto c = dyn_cast<cf::CondBranchOp>(op)) {
                    condBr = c;
                    break;
                }
                if (auto b = dyn_cast<cf::BranchOp>(op)) {
                    br = b;
                    break;
                }
            }

            if (condBr) {
                // Fork: add both branch destinations to forbidden set
                llvm::DenseSet<Block *> innerForbidden = condBrTargets;
                innerForbidden.insert(condBr.getTrueDest());
                innerForbidden.insert(condBr.getFalseDest());

                auto trueResult =
                        traverseCFGAndSchedule(condBr.getTrueDest(), visited, tasks, blockToMilpBlock, blockDependences,
                                               qubitInits, qubitMeas, prereqs, innerForbidden);

                auto falseResult = traverseCFGAndSchedule(condBr.getFalseDest(), std::move(visited), std::move(tasks),
                                                          blockToMilpBlock, blockDependences, qubitInits, qubitMeas,
                                                          prereqs, std::move(innerForbidden));

                // Pick worse path (longer execution time = worse fidelity)
                return (trueResult.globalTime >= falseResult.globalTime) ? trueResult : falseResult;
            }

            if (br) {
                block = br.getDest();
                continue;
            }

            // No explicit branch: find next ready block via blk_meta prerequisites
            // Not a cf.br, so clear the tracking —> ordering comes from blk_meta only
            prevBlockLastTask.clear();
            block = scanForReadyBlock(*startBlock->getParent(), visited, prereqs, condBrTargets);
        }

        return runScheduler(std::move(tasks), qubitInits, qubitMeas);
    }

    /**
     * Compute qubit lifetimes.
     * Use precedences between blocks.
     * Qpu tasks are split around initialization and measurement operations.
     * Given information about execution time of each operation inside tasks,
     * estimate the parallel scheduling of cpu and qpu tasks, keeping track of the
     * execution time. The qubits lifetimes can then be computed from the time
     * at which their initialization task was scheduled and the time at which their
     * measurement task is scheduled.
     */
    QoalaHostQubitLifetime::QoalaHostQubitLifetime(Operation *op) {
        LLVM_DEBUG(llvm::dbgs() << "Running QoalaHostQubitLifetimePass\n");

        // Current implementation is tightly coupled with fidelity estimation, i.e.,
        // lifetime is tracked up to measurement or last wto-qubit op.
        // In future could change to track every qubit up to last quantum op.

        // Comply with mlir assumptions on analisys passes.
        // Clone operation to avoid modifying the actual IR.
        Operation *clonedOp = op->clone();
        auto module = dyn_cast<ModuleOp>(*clonedOp);
        auto mainFuncs = module.getOps<qoalahost::MainFuncOp>();

        assert(!mainFuncs.empty() && "No main function found in module.");

        qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();

        llvm::StringMap<Operation *> routineMap = reordering::collectRoutineMap(module);

        auto [blocks, opToMilpOp, _, unresolvedEdges, idToBlockMap, blocksStatus] =
                reordering::buildMilpBlocks(mainFunc, routineMap);
        assert(!failed(blocksStatus) && "Failed to build predences.");
        assert(unresolvedEdges.empty() && "Unresolved precedence edges detected.");

        LLVM_DEBUG(llvm::dbgs() << "Blocks and tasks:\n");
        for (const auto &bp : blocks) {
            const auto *b = bp.get();
            LLVM_DEBUG(llvm::dbgs() << " - Block " << b->getId() << " (type=" << static_cast<int>(b->getType())
                                    << ")\n");
            for (const auto &tPtr : b->getTasks()) {
                const auto *t = tPtr.get();
                LLVM_DEBUG(llvm::dbgs() << "    * Task " << t->getId() << " [Group=" << static_cast<int>(t->getGroup())
                                        << "]\n");
                for (const auto *operation : t->getOperations()) {
                    LLVM_DEBUG(llvm::dbgs()
                               << "        - " << operation->getId() << " => " << operation->getOperation()->getName()
                               << " , duration=" << operation->getDuration() << "ns\n");
                }
            }
        }

        auto [qubitToOps, collStatus] = reordering::collectQubitUsage(mainFunc, module);
        assert(!failed(collStatus) && "Could not collect qubit usage.");

        std::vector<std::shared_ptr<LiveQubit>> qubits = getQubitCriticalOps(qubitToOps, opToMilpOp);

        LLVM_DEBUG(llvm::dbgs() << "Constructed LiveQubits:\n");
        for (const auto &q : qubits) {
            LLVM_DEBUG(llvm::dbgs() << " - " << q->getId() << ": alloc=");
            LLVM_DEBUG(llvm::dbgs() << (q->getAllocation() ? q->getAllocation()->getId() : "null") << ", meas=");
            LLVM_DEBUG(llvm::dbgs() << (q->getMeasurement() ? q->getMeasurement()->getId() : "null")
                                    << ", lastTwoQubitOp=");
            LLVM_DEBUG(llvm::dbgs() << (q->getTwoQubitOp() ? q->getTwoQubitOp()->getId() : "null") << "\n");
        }

        // Build Block* -> MILPBlock* map for CFG traversal
        llvm::DenseMap<mlir::Block *, reordering::MILPBlock *> blockToMilpBlock;
        for (const auto &bp : blocks) {
            if (bp->getBlock()) {
                blockToMilpBlock[bp->getBlock()] = bp.get();
            }
        }

        std::unordered_map<std::string, std::string> qubitInits;
        std::unordered_map<std::string, std::string> qubitMeas;

        buildQubitMaps(qubits, qubitInits, qubitMeas);

        // Build prerequisite maps from blk_meta attributes.
        BlockPrerequisites prereqs = buildBlockPrerequisites(mainFunc);

        // Convert prerequisites to MILPBlock-level dependencies for task scheduling.
        auto blockDependences = convertPrereqsToMilpDeps(prereqs, blockToMilpBlock);

        // Traverse blocks with branch awareness: at conditional branches,
        // evaluate both paths and pick the worst case (longest execution time).
        Block &entryBlock = mainFunc.front();
        SchedulerResult result = traverseCFGAndSchedule(&entryBlock, llvm::DenseSet<mlir::Block *>{}, BranchTasks{},
                                                        blockToMilpBlock, blockDependences, qubitInits, qubitMeas,
                                                        prereqs, llvm::DenseSet<mlir::Block *>{});

        qubitLifetimes = result.qubitLifetimes;

        // Erase cloned operation.
        clonedOp->erase();
    }
} // namespace qoala::analysis::qubitlife
