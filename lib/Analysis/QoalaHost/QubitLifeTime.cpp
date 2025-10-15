#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "mlir/IR/Operation.h"

#define DEBUG_TYPE "qoalahost-qubit-life"

using namespace mlir;
using namespace llvm;
using namespace qoala::dialects;

namespace qoala::analysis::qbitlife {

    QoalaHostQubitLifeTime::QoalaHostQubitLifeTime(Operation *op) {
        /**
         * Compute qubit life times.
         * Use predecences between block given by reordering pass.
         * Qpu tasks are split around initialization and measurement operations.
         * Given information about execution time of each operation inside tasks,
         * estimate the parallel scheduling of cpu and qpu tasks, keeping track of the
         * execution time. The qubits lifetimes can then be computed from the time
         * at which their initialization task was scheduled and the time at wich their
         * measurement task is scheduled.
         */
        LLVM_DEBUG(llvm::dbgs() << "Running QoalaHostQubitLifeTimePass\n");
        auto module = dyn_cast<ModuleOp>(*op);
        auto [blocks, qubits, precedences, _, __] = reordering::buildMILPFromMLIR(module);

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
         * There is also an absolute time that increase
         * every time a task is scheduled.
         * A task is scheduled only if tis exectuion time would not increase
         * it's relative time above the absolute time.
         * This enable the evaluation of parallel exectuion
         * of classical and quantum tasks.
         * Qubit lifetimes are given by the difference between
         * the relative time at wich a neasurement task is scheduled
         * and the time at which the initialization task was scheduled.
         * We can also estimate total execution time at the end.
         */

        int cpuTime = 0;
        int qpuTime = 0;
        int globalTime = 0;

        qubitLifeTimes.reserve(qubits.size());

        LLVM_DEBUG(llvm::dbgs() << "\n=== Starting Task Scheduling ===\n");

        while (!cpuTasks.empty() || !qpuTasks.empty()) {

            // Find available tasks
            auto nextQpuTaskIdx = findNextAvailableTask(qpuTasks, taskDependences);
            auto nextCpuTaskIdx = findNextAvailableTask(cpuTasks, taskDependences);

            // Compute global time increment
            int timeIncrement = computeNextTimeIncrement(cpuTasks, qpuTasks, nextCpuTaskIdx, nextQpuTaskIdx, cpuTime,
                                                         qpuTime, globalTime);

            globalTime += timeIncrement;
            LLVM_DEBUG(llvm::dbgs() << "Global Time: " << globalTime << "\n");

            // Schedule qpu task if ready
            scheduleTaskIfReady(qpuTasks, nextQpuTaskIdx, qpuTime, globalTime, taskDependences, qubitInits, qubitMeas,
                                qubitLifeTimes);

            // Schedule cpu task if ready
            scheduleTaskIfReady(cpuTasks, nextCpuTaskIdx, cpuTime, globalTime, taskDependences, qubitInits, qubitMeas,
                                qubitLifeTimes);
        }

        // TODO check for not measured qubits

        LLVM_DEBUG(llvm::dbgs() << "\n=== Qubit Lifetimes ===\n");
        for (const auto &[qubitId, lifetime] : qubitLifeTimes) {
            LLVM_DEBUG(llvm::dbgs() << qubitId << ": " << lifetime << "\n");
        }
        LLVM_DEBUG(llvm::dbgs() << "\nTotal Execution Time: " << globalTime << "\n");
    }

    /**
     * Process a block and separate qpu and cpu tasks.
     * Track dependences betwenn tasks.
     * Register qubit initialization and measurement tasks.
     */
    void QoalaHostQubitLifeTime::processBlock(
            const reordering::MILPBlock *block,
            const std::unordered_map<std::string, std::vector<reordering::MILPBlock *>> &blockDependences,
            std::unordered_map<std::string, std::vector<std::string>> &taskDependences, std::vector<Task> &qpuTasks,
            std::vector<Task> &cpuTasks, const std::unordered_map<std::string, std::string> &qubitInits,
            const std::unordered_map<std::string, std::string> &qubitMeas,
            std::unordered_map<std::string, std::vector<std::string>> &qubitInitsMeas) {

        std::string intraBlockPred;
        std::string last;

        auto depIt = blockDependences.find(block->getId());
        bool interBlockPred = (depIt != blockDependences.end());

        LLVM_DEBUG(llvm::dbgs() << "Block '" << block->getId() << "' has inter block dependency: " << interBlockPred
                                << "\n");

        for (const auto &tPtr : block->getTasks()) {
            const auto *t = tPtr.get();

            int taskTime = 0;

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

    void
    QoalaHostQubitLifeTime::buildQubitMaps(const std::vector<std::shared_ptr<reordering::MILPQubit>> &qubits,
                                           std::unordered_map<std::string, std::string> &qubitInits,
                                           std::unordered_map<std::string, std::string> &qubitMeas,
                                           std::unordered_map<std::string, std::vector<std::string>> &qubitsInitMeas) {

        qubitInits.reserve(qubits.size());
        qubitMeas.reserve(qubits.size());
        qubitsInitMeas.reserve(qubits.size());

        for (const auto &qubit : qubits) {
            const std::string &qubitId = qubit->getId();
            const std::string &allocId = qubit->getAllocation()->getId();
            const std::string &measId = qubit->getMeasurement()->getId();

            qubitInits.emplace(allocId, qubitId);
            qubitMeas.emplace(measId, qubitId);

            LLVM_DEBUG(llvm::dbgs() << "Qubit '" << qubitId << "' initialized in '" << allocId << "' and measured in '"
                                    << measId << "'.\n");
        }
    }

    void QoalaHostQubitLifeTime::buildBlockDependencies(
            const std::vector<std::pair<reordering::MILPBlock *, reordering::MILPBlock *>> &precedences,
            std::unordered_map<std::string, std::vector<reordering::MILPBlock *>> &blockDependences) {

        for (const auto &[predecessor, successor] : precedences) {
            blockDependences[successor->getId()].push_back(predecessor);
        }
    }

    /**
     * Verify if a task is available for scheduling.
     * A task is available if all its dependences have already been scheduled.
     */
    bool QoalaHostQubitLifeTime::isTaskAvailable(
            const std::string &taskName,
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
    std::optional<size_t> QoalaHostQubitLifeTime::findNextAvailableTask(
            const std::vector<Task> &tasks,
            const std::unordered_map<std::string, std::vector<std::string>> &taskDependences) {

        for (size_t i = 0; i < tasks.size(); ++i) {
            if (isTaskAvailable(tasks[i].name, taskDependences)) {
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
    void QoalaHostQubitLifeTime::updateQubitLifetime(const Task &scheduledTask, int currentTime,
                                                     const std::unordered_map<std::string, std::string> &qubitInits,
                                                     const std::unordered_map<std::string, std::string> &qubitMeas,
                                                     std::unordered_map<std::string, int> &qubitLifeTimes) {

        // Check if it is an init task
        auto initIt = qubitInits.find(scheduledTask.name);
        if (initIt != qubitInits.end()) {
            qubitLifeTimes.emplace(initIt->second, currentTime - scheduledTask.time);
            LLVM_DEBUG(llvm::dbgs() << "Qubit '" << initIt->second << "' initialized at time "
                                    << (currentTime - scheduledTask.time) << "\n");
            return;
        }

        // Check if it is a measurement task
        auto measIt = qubitMeas.find(scheduledTask.name);
        if (measIt != qubitMeas.end()) {
            auto lifetimeIt = qubitLifeTimes.find(measIt->second);
            if (lifetimeIt != qubitLifeTimes.end()) {
                int initTime = lifetimeIt->second;
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
    bool QoalaHostQubitLifeTime::scheduleTaskIfReady(
            std::vector<Task> &tasks, std::optional<size_t> taskIndex, int &taskTime, int globalTime,
            std::unordered_map<std::string, std::vector<std::string>> &taskDependences,
            const std::unordered_map<std::string, std::string> &qubitInits,
            const std::unordered_map<std::string, std::string> &qubitMeas,
            std::unordered_map<std::string, int> &qubitLifeTimes) {

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
        if (globalTime - taskTime < task.time) {
            return false;
        }

        LLVM_DEBUG(llvm::dbgs() << "Scheduling Task: " << task.name << " at time " << globalTime << "\n");

        // Update qubit lifetime if needed
        updateQubitLifetime(task, globalTime, qubitInits, qubitMeas, qubitLifeTimes);

        // Update the state
        taskTime = globalTime;

        // Remove the task
        taskDependences.erase(task.name);
        tasks.erase(tasks.begin() + *taskIndex);

        return true;
    }

    /**
     * Compute the next global time increment.
     */
    int QoalaHostQubitLifeTime::computeNextTimeIncrement(const std::vector<Task> &cpuTasks,
                                                         const std::vector<Task> &qpuTasks,
                                                         std::optional<size_t> nextCpuTaskIdx,
                                                         std::optional<size_t> nextQpuTaskIdx, int cpuTime, int qpuTime,
                                                         int currentTime) {

        bool hasCpuTask = nextCpuTaskIdx.has_value() && nextCpuTaskIdx < cpuTasks.size();
        bool hasQpuTask = nextQpuTaskIdx.has_value() && nextQpuTaskIdx < qpuTasks.size();

        if (!hasCpuTask && !hasQpuTask) {
            return 0;
        }

        if (!hasCpuTask) {
            return qpuTasks[*nextQpuTaskIdx].time - (currentTime - qpuTime);
        }

        if (!hasQpuTask) {
            return cpuTasks[*nextCpuTaskIdx].time - (currentTime - cpuTime);
        }

        /**
         * If both cpu and qpu tasks are found, select the one with the shortes execution time
         * This enables to keep track of parallel cpu and qpu tasks execution,
         * where first the shorter cpu tasks are scheduled, up until no other cpu tasks are avialable
         * or the globel time is enough to fit in the qpu tasks (now scheduled in parallel with all
         * the already scheduled cpu tasks).
         */
        int cpuIncrement = cpuTasks[*nextCpuTaskIdx].time - (currentTime - cpuTime);
        int qpuIncrement = qpuTasks[*nextQpuTaskIdx].time - (currentTime - qpuTime);
        return std::min(cpuIncrement, qpuIncrement);
    }

    // Return computed qubit life times.
    std::unordered_map<std::string, int> QoalaHostQubitLifeTime::getLifeTimes() const { return this->qubitLifeTimes; }

} // namespace qoala::analysis::qbitlife
