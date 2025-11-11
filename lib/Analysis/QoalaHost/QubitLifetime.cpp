#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "mlir/IR/AsmState.h"
#include "mlir/IR/Operation.h"

#define DEBUG_TYPE "qoalahost-qubit-life"

using namespace mlir;
using namespace llvm;
using namespace qoala::dialects;
using namespace qoala::analysis;

namespace qoala::analysis::qubitlife {

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
                        const std::unordered_map<mlir::Operation *, reordering::MILPOperation *> &opToMilpOp) {
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
                /**
                 * Treat last two-qubit op as a measure op.
                 * A measure op will overwrite the last two-qubit op.
                 */
                if (llvm::isa<netqasm::CnotOp, netqasm::CzOp, netqasm::CrotXOp>(op)) {
                    auto itTwoQubitOp = opToMilpOp.find(op);
                    twoQubitOp = (itTwoQubitOp != opToMilpOp.end()) ? itTwoQubitOp->second : nullptr;
                }
            }

            if (allocOp == nullptr) {
                llvm::outs() << "Missing Alloc Op for qubit: " << entry.first << ".\n";
                assert(false);
            }

            std::string id = allocOp->getId().substr(6);
            std::shared_ptr<LiveQubit> qubitPtr = std::make_shared<LiveQubit>(id);

            // Attach known alloc/meas to the qubit model object
            if (allocOp) {
                qubitPtr->setAllocation(allocOp);
            }
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
                               std::unordered_map<std::string, std::string> &qubitMeas,
                               std::unordered_map<std::string, std::vector<std::string>> &qubitsInitMeas) {

        qubitInits.reserve(qubits.size());
        qubitMeas.reserve(qubits.size());
        qubitsInitMeas.reserve(qubits.size());

        for (const auto &qubit : qubits) {
            const std::string &qubitId = qubit->getId();
            if (qubit->getAllocation() == nullptr) {
                llvm::outs() << "Alloc Op not found for qubit " << qubitId << ".\n";
                assert(false);
            }
            const std::string &allocId = qubit->getAllocation()->getId();
            auto measPtr = qubit->getMeasurement();
            if (measPtr == nullptr) {
                measPtr = qubit->getTwoQubitOp();
            }

            if (measPtr != nullptr) {
                qubitInits.emplace(allocId, qubitId);
                const std::string &measId = measPtr->getId();
                qubitMeas.emplace(measId, qubitId);
                LLVM_DEBUG(llvm::dbgs() << "Qubit '" << qubitId << "' initialized in '" << allocId
                                        << "' and measured in '" << measId << "'.\n");
            }
        }
    }

    static void
    buildBlockDependencies(const std::vector<std::pair<reordering::MILPBlock *, reordering::MILPBlock *>> &precedences,
                           std::unordered_map<std::string, std::vector<reordering::MILPBlock *>> &blockDependences) {

        for (const auto &[predecessor, successor] : precedences) {
            blockDependences[successor->getId()].push_back(predecessor);
        }
    }

    /**
     * Process a block and separate qpu and cpu tasks.
     * Track dependences between tasks.
     * Register qubit initialization and measurement tasks.
     */
    static void
    processBlock(const reordering::MILPBlock *block,
                 const std::unordered_map<std::string, std::vector<reordering::MILPBlock *>> &blockDependences,
                 std::unordered_map<std::string, std::vector<std::string>> &taskDependences,
                 std::vector<Task> &qpuTasks, std::vector<Task> &cpuTasks,
                 const std::unordered_map<std::string, std::string> &qubitInits,
                 const std::unordered_map<std::string, std::string> &qubitMeas,
                 std::unordered_map<std::string, std::vector<std::string>> &qubitsInitMeas) {

        std::string intraBlockPred;
        std::string last;

        auto depIt = blockDependences.find(block->getId());
        bool interBlockPred = (depIt != blockDependences.end());

        LLVM_DEBUG(llvm::dbgs() << "Block '" << block->getId() << "' has inter block dependency: " << interBlockPred
                                << "\n");

        for (const auto &tPtr : block->getTasks()) {
            const auto *t = tPtr.get();

            uint32_t taskTime = 0;

            for (const auto *op : t->getOperations()) {
                taskTime += op->getDuration();

                const std::string &opId = op->getId();
                auto initIt = qubitInits.find(opId);
                auto measIt = qubitMeas.find(opId);

                if (initIt != qubitInits.end() || measIt != qubitMeas.end()) {
                    qpuTasks.emplace_back(Task(opId, taskTime));
                    taskDependences.try_emplace({opId, {}});

                    if (initIt != qubitInits.end()) {
                        qubitsInitMeas.emplace(initIt->second, std::vector<std::string>{opId});
                    } else {
                        qubitsInitMeas.at(measIt->second).push_back(opId);
                    }

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
                    qpuTasks.emplace_back(Task(last, taskTime));
                } else {
                    cpuTasks.emplace_back(Task(last, taskTime));
                }
                taskDependences.insert({last, {}});
                LLVM_DEBUG(llvm::dbgs() << "Added Task '" << last << "' with execution time: " << taskTime << ".\n");
                taskTime = 0;
                if (!intraBlockPred.empty()) {
                    LLVM_DEBUG(llvm::dbgs() << "Task has intra block dependency with '" << intraBlockPred << "'.\n");
                    taskDependences.at(last).push_back(intraBlockPred);
                }
                intraBlockPred = last;
            }
        }
        if (interBlockPred == true) {
            for (const auto &temp : depIt->second) {
                auto lastBlockTask = temp->getTasks().back()->getOperations().back()->getId();
                auto currentBlockTask = block->getTasks().front()->getOperations().front()->getId();
                LLVM_DEBUG(llvm::dbgs() << "Task " << currentBlockTask << " has inter block dependency with '"
                                        << lastBlockTask << "'.\n");
                taskDependences.at(currentBlockTask).push_back(lastBlockTask);
            }
        }
    }

    /**
     * Verify if a task is available for scheduling.
     * A task is available if all its dependences have already been scheduled.
     */
    static bool isTaskAvailable(const std::string &taskName,
                                const std::unordered_map<std::string, std::vector<std::string>> &taskDependences) {
        auto it = taskDependences.find(taskName);
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
    static void updateQubitLifetime(const Task &scheduledTask, uint32_t currentTime,
                                    const std::unordered_map<std::string, std::string> &qubitInits,
                                    const std::unordered_map<std::string, std::string> &qubitMeas,
                                    std::unordered_map<std::string, uint32_t> &qubitLifeTimes) {

        // Check if it is an init task
        auto initIt = qubitInits.find(scheduledTask.getName());
        if (initIt != qubitInits.end()) {
            // Life time should start after initialization is completed
            qubitLifeTimes.emplace(initIt->second, currentTime);
        }

        // Check if it is a measurement task
        auto measIt = qubitMeas.find(scheduledTask.getName());
        if (measIt != qubitMeas.end()) {
            auto lifetimeIt = qubitLifeTimes.find(measIt->second);
            if (lifetimeIt != qubitLifeTimes.end()) {
                uint32_t initTime = lifetimeIt->second;
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
    static bool scheduleTaskIfReady(std::vector<Task> &tasks, std::optional<size_t> taskIndex, uint32_t &taskTime,
                                    uint32_t globalTime,
                                    std::unordered_map<std::string, std::vector<std::string>> &taskDependences,
                                    const std::unordered_map<std::string, std::string> &qubitInits,
                                    const std::unordered_map<std::string, std::string> &qubitMeas,
                                    std::unordered_map<std::string, uint32_t> &qubitLifeTimes) {

        if (!taskIndex.has_value()) {
            taskTime = globalTime;
            return false;
        }

        const Task &task = tasks[*taskIndex];

        /**
         * A task is ready and can be scheduled only if
         * its execution time would not increase the time
         * of its task type beyond the global time.
         */
        if (globalTime - taskTime < task.getTime()) {
            return false;
        }

        LLVM_DEBUG(llvm::dbgs() << "Scheduling Task: " << task.getName() << " at time " << globalTime << "\n");

        // Update qubit lifetime if needed
        updateQubitLifetime(task, globalTime, qubitInits, qubitMeas, qubitLifeTimes);

        // Update the state
        taskTime = globalTime;

        // Remove the task
        taskDependences.erase(task.getName());
        tasks.erase(tasks.begin() + *taskIndex);

        return true;
    }

    /**
     * Compute the next global time increment.
     */
    static uint32_t computeNextTimeIncrement(const std::vector<Task> &cpuTasks, const std::vector<Task> &qpuTasks,
                                             std::optional<size_t> nextCpuTaskIdx, std::optional<size_t> nextQpuTaskIdx,
                                             uint32_t cpuTime, uint32_t qpuTime, uint32_t currentTime) {

        bool hasCpuTask = nextCpuTaskIdx.has_value() && *nextCpuTaskIdx < cpuTasks.size();
        bool hasQpuTask = nextQpuTaskIdx.has_value() && *nextQpuTaskIdx < qpuTasks.size();

        if (!hasCpuTask && !hasQpuTask) {
            return 0;
        }

        if (!hasCpuTask) {
            return qpuTasks[*nextQpuTaskIdx].getTime() - (currentTime - qpuTime);
        }

        if (!hasQpuTask) {
            return cpuTasks[*nextCpuTaskIdx].getTime() - (currentTime - cpuTime);
        }

        /**
         * If both cpu and qpu tasks are found, select the one with the shortes execution time
         * This enables to keep track of parallel cpu and qpu tasks execution,
         * where first the shorter cpu tasks are scheduled, up until no other cpu tasks are avialable
         * or the global time is enough to fit in the qpu tasks (now scheduled in parallel with all
         * the already scheduled cpu tasks).
         */
        uint32_t cpuIncrement = cpuTasks[*nextCpuTaskIdx].getTime() - (currentTime - cpuTime);
        uint32_t qpuIncrement = qpuTasks[*nextQpuTaskIdx].getTime() - (currentTime - qpuTime);
        return std::min(cpuIncrement, qpuIncrement);
    }

    QoalaHostQubitLifetime::QoalaHostQubitLifetime(Operation *op) {
        /**
         * Compute qubit life times.
         * Use predecences between block given by reordering pass.
         * Qpu tasks are split around initialization and measurement operations.
         * Given information about execution time of each operation inside tasks,
         * estimate the parallel scheduling of cpu and qpu tasks, keeping track of the
         * execution time. The qubits lifetimes can then be computed from the time
         * at which their initialization task was scheduled and the time at which their
         * measurement task is scheduled.
         */
        LLVM_DEBUG(llvm::dbgs() << "Running QoalaHostQubitLifetimePass\n");

        /**
         * TODO Branching will create forks in the control flow and
         * current implementation will see the branches as
         * concurrent tasks, to be scheduled sequentially.
         * We should instead differentiate between normal control flow and branching,
         * and only schedule the longest of the branches for worst case scenario estimation.
         */

        // Comply with mlir assumptions on analisys passes.
        // Clone operation to avoid modifying the actual IR.
        Operation *clonedOp = op->clone();
        auto module = dyn_cast<ModuleOp>(*clonedOp);
        auto mainFuncs = module.getOps<qoalahost::MainFuncOp>();

        assert(!mainFuncs.empty() && "No main function found in module.");

        qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();

        llvm::StringMap<Operation *> routineMap = reordering::collectRoutineMap(module);

        auto [blocks, opToMilpOp, precedences, unresolvedEdges, _, blocksStatus] =
                reordering::buildMilpBlocks(mainFunc, routineMap);
        assert(!failed(blocksStatus) && "Falied to build predences.");
        assert(unresolvedEdges.empty() && "Unresolved precedence edges detected.");

        LLVM_DEBUG(llvm::dbgs() << "Blocks and tasks:\n");
        for (const auto &bp : blocks) {
            const auto *b = bp.get();
            LLVM_DEBUG(llvm::dbgs() << " - Block " << b->getId() << " (type=" << (int) b->getType() << ")\n");
            for (const auto &tPtr : b->getTasks()) {
                const auto *t = tPtr.get();
                LLVM_DEBUG(llvm::dbgs() << "    * Task " << t->getId() << " [Group=" << (int) t->getGroup() << "]\n");
                for (const auto *op : t->getOperations()) {
                    LLVM_DEBUG(llvm::dbgs() << "        - " << op->getId() << " => " << op->getOperation()->getName()
                                            << " , duration=" << op->getDuration() << "ns\n");
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

        std::vector<Task> cpuTasks;
        std::vector<Task> qpuTasks;
        std::unordered_map<std::string, std::vector<std::string>> taskDependences;

        std::unordered_map<std::string, std::string> qubitInits;
        std::unordered_map<std::string, std::string> qubitMeas;
        std::unordered_map<std::string, std::vector<reordering::MILPBlock *>> blockDependences;

        buildQubitMaps(qubits, qubitInits, qubitMeas, qubitsInitMeas);
        buildBlockDependencies(precedences, blockDependences);

        for (const auto &bp : blocks) {
            processBlock(bp.get(), blockDependences, taskDependences, qpuTasks, cpuTasks, qubitInits, qubitMeas,
                         qubitsInitMeas);
        }

        /**
         * Traverse cpuTasks and qpuTasks in parallel.
         * Find the next cTask and qTask that could be scheduled, i.e,
         * all its dependences have already been scheduled.
         * cpuTasks and qpuTasks have a relative time that increase only when
         * a cTask or qTask is scheduled respectevely.
         * There is also a global time that increases
         * every time a task is scheduled.
         * A task is scheduled only if its exectuion time would not increase
         * it's relative time above the absolute time.
         * This enable the evaluation of parallel exectuion
         * of classical and quantum tasks.
         * Qubit lifetimes are given by the difference between
         * the relative time at wich a measurement task is scheduled
         * and the time at which the initialization task was scheduled.
         * We can also estimate total execution time at the end.
         */

        uint32_t cpuTime = 0;
        uint32_t qpuTime = 0;
        uint32_t globalTime = 0;

        qubitLifeTimes.reserve(qubits.size());

        LLVM_DEBUG(llvm::dbgs() << "\n=== Starting Task Scheduling ===\n");

        while (!cpuTasks.empty() || !qpuTasks.empty()) {

            // Find available tasks
            auto nextQpuTaskIdx = findNextAvailableTask(qpuTasks, taskDependences);
            auto nextCpuTaskIdx = findNextAvailableTask(cpuTasks, taskDependences);

            // Compute global time increment
            uint32_t timeIncrement = computeNextTimeIncrement(cpuTasks, qpuTasks, nextCpuTaskIdx, nextQpuTaskIdx,
                                                              cpuTime, qpuTime, globalTime);

            globalTime += timeIncrement;
            LLVM_DEBUG(llvm::dbgs() << "Global Time: " << globalTime << "\n");

            // Schedule qpu task if ready
            scheduleTaskIfReady(qpuTasks, nextQpuTaskIdx, qpuTime, globalTime, taskDependences, qubitInits, qubitMeas,
                                qubitLifeTimes);

            // Schedule cpu task if ready
            scheduleTaskIfReady(cpuTasks, nextCpuTaskIdx, cpuTime, globalTime, taskDependences, qubitInits, qubitMeas,
                                qubitLifeTimes);
        }

        LLVM_DEBUG(llvm::dbgs() << "\n=== Qubit Lifetimes ===\n");
        for (const auto &[qubitId, lifetime] : qubitLifeTimes) {
            LLVM_DEBUG(llvm::dbgs() << qubitId << ": " << lifetime << "\n");
        }
        LLVM_DEBUG(llvm::dbgs() << "\nTotal Execution Time: " << globalTime << "\n");

        // Erase cloned operation.
        clonedOp->erase();
    }

    // Return computed qubit life times.
    std::unordered_map<std::string, uint32_t> QoalaHostQubitLifetime::getLifetimes() const { return qubitLifeTimes; }

} // namespace qoala::analysis::qubitlife
