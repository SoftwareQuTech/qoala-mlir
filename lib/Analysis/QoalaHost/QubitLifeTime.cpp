#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "mlir/IR/Operation.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include <queue>

#define DEBUG_TYPE "qoalahost-qubit-life"

using namespace mlir;
using namespace llvm;
using namespace qoala::dialects;

namespace qoala::analysis::qbitlife {

    QoalaHostQubitLifeTime::QoalaHostQubitLifeTime(Operation *op) {
        LLVM_DEBUG(llvm::dbgs() << "Running QoalaHostQubitLifeTimePass\n");
        auto module = dyn_cast<ModuleOp>(*op);
        auto [blocks, qubits, precedences, _, __] = reordering::buildMILPFromMLIR(module);

        std::vector<Task> cpuTasks;
        std::vector<Task> qpuTasks;
        std::unordered_map<std::string, std::vector<std::string>> taskDependences;

        std::unordered_map<std::string, std::string> qubitInits;
        std::unordered_map<std::string, std::string> qubitMeas;
        std::unordered_map<std::string, std::vector<reordering::MILPBlock*>> blockDependences;

        qubitInits.reserve(qubits.size());
        qubitMeas.reserve(qubits.size());
        qubitsInitMeas.reserve(qubits.size());

        for (const auto &q : qubits) {
            qubitInits.emplace(q->getAllocation()->getId(), q->getId());
            qubitMeas.emplace(q->getMeasurement()->getId(), q->getId());
            LLVM_DEBUG(llvm::dbgs() << "Qubit '" << q->getId() 
                        << "' initialized in '" << q->getAllocation()->getId() 
                        << "' and measured in '" << q->getMeasurement()->getId() << "'.\n");
        }

        for (auto &e : precedences) {
            if (blockDependences.find(e.second->getId()) == blockDependences.end()) {
                blockDependences.insert({e.second->getId(), {}});
            }
            blockDependences.at(e.second->getId()).push_back(e.first);
        }

        bool interBlockPred = false;

        for (const auto &bp : blocks) {
            const auto *b = bp.get();
            std::string intraBlockPred = "";
            std::string last;

            auto depIt = blockDependences.find(b->getId());
            interBlockPred = (depIt != blockDependences.end());
            LLVM_DEBUG(llvm::dbgs() << "Block '" << b->getId() << "' has inter block dependency: " << interBlockPred << "\n");

            for (const auto &tPtr : b->getTasks()) {
                const auto *t = tPtr.get();
                
                int taskTime = 0;

                for (const auto *op : t->getOperations()) {       
                    taskTime += op->getDuration();

                    const std::string& opId = op->getId();
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

                        LLVM_DEBUG(llvm::dbgs() << "Added Task '" << opId << "' with execution time: " << taskTime << ".\n");
                        
                        taskTime = 0;

                        if (!intraBlockPred.empty()) {
                            LLVM_DEBUG(llvm::dbgs() << "Task has intra block dependency with '" << intraBlockPred << "'.\n");
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
                for (auto const&temp: depIt->second) {
                    auto lastBlockTask = temp->getTasks().back()->getOperations().back()->getId();
                    auto currentBlockTask = b->getTasks().front()->getOperations().front()->getId();
                    LLVM_DEBUG(llvm::dbgs() << "Task " << currentBlockTask << " has inter block dependency with '" << lastBlockTask << "'.\n");
                    taskDependences.at(currentBlockTask).push_back(lastBlockTask);
                }
                interBlockPred = false;
            }
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

        Task lastCpuTask = Task();
        Task lastQpuTask = Task();
        std::optional<int> nextCpuTask;
        std::optional<int> nextQpuTask;

        int cpuTime = 0;
        int qpuTime = 0;
        int time = 0;
        
        qubitLifeTimes.reserve(qubits.size());

        do {

            bool available = true;
            for (size_t i = 0; i < qpuTasks.size(); i++) {
                const auto& qTask = qpuTasks[i];
                const auto& deps = taskDependences[qTask.name];
                for (size_t dep_idx = 0; dep_idx < deps.size() && available; ++dep_idx) {
                    available = (taskDependences.find(deps[dep_idx]) == taskDependences.end());
                }

                if (available) {
                    nextQpuTask = i;
                    break;
                }
                available = true;
            }

            available = true;

            for (size_t i = 0; i < cpuTasks.size(); i++) {
                const auto& cTask = cpuTasks[i];
                const auto& deps = taskDependences[cTask.name];
                for (size_t dep_idx = 0; dep_idx < deps.size() && available; ++dep_idx) {
                    available = (taskDependences.find(deps[dep_idx]) == taskDependences.end());
                }

                if (available) {
                    nextCpuTask = i;
                    break;
                }
                available = true;
            }

            if (!nextQpuTask && !cpuTasks.empty()) {
                lastQpuTask = cpuTasks[*nextCpuTask];
            }

            if (!nextCpuTask && !qpuTasks.empty()) {
                lastCpuTask = qpuTasks[*nextQpuTask];
            }

            if (!nextCpuTask || !nextQpuTask) {
                if (!nextCpuTask) {
                    time += qpuTasks[*nextQpuTask].time-time+qpuTime;
                    cpuTime = time;
                } else {
                    time += cpuTasks[*nextCpuTask].time-time+cpuTime;
                    qpuTime = time;
                }
            } else {
                time += std::min(qpuTasks[*nextQpuTask].time-time+qpuTime, cpuTasks[*nextCpuTask].time-time+cpuTime);
            }

            if (nextQpuTask && time-qpuTime >= qpuTasks[*nextQpuTask].time) {
                auto qTask = qpuTasks[*nextQpuTask];
                LLVM_DEBUG(llvm::dbgs() << "Found Next Quantum Task: " << qTask.name << "\n");
                for (auto dep: taskDependences.at(qTask.name)) {
                    if (dep == lastQpuTask.name) {
                        lastQpuTask.reset();
                    }
                }
                lastQpuTask = qpuTasks[*nextQpuTask];
                qpuTasks.erase(qpuTasks.begin() + *nextQpuTask);
                taskDependences.erase(qTask.name);

                qpuTime = time;

                auto initIt = qubitInits.find(qTask.name);
                
                if (initIt != qubitInits.end()) {
                    qubitLifeTimes.emplace(initIt->second, time-qTask.time);
                } else {
                    auto measIt = qubitMeas.find(qTask.name);
                    if (measIt != qubitMeas.end()) {
                        qubitLifeTimes.at(measIt->second) = time - qubitLifeTimes.at(measIt->second);
                    }
                }

            }

            if (nextCpuTask && time-cpuTime >= cpuTasks[*nextCpuTask].time) {
                auto cTask = cpuTasks[*nextCpuTask];
                LLVM_DEBUG(llvm::dbgs() << "Found Next Classical Task: " << cTask.name << "\n");
                for (auto dep: taskDependences.at(cTask.name)) {
                    if (dep == lastCpuTask.name) {
                        lastCpuTask.reset();
                    }
                }

                lastCpuTask = cpuTasks[*nextCpuTask];
                cpuTasks.erase(cpuTasks.begin() + *nextCpuTask);
                taskDependences.erase(cTask.name);

                cpuTime = time;
            }

            LLVM_DEBUG(llvm::dbgs() << "TIME: " << time << "\n");

            available = true;
            nextCpuTask.reset();
            nextQpuTask.reset();
        }
        while (!cpuTasks.empty() || !qpuTasks.empty());
        
        LLVM_DEBUG(llvm::dbgs() << "Life Times:\n");
        for (const auto& [qubit, lifeTime] : qubitLifeTimes) {
            LLVM_DEBUG(llvm::dbgs() << qubit << ": " << lifeTime << "\n");
        }
        LLVM_DEBUG(llvm::dbgs() << "Execution Time: " << time << ".\n");

    }

    std::unordered_map<std::string, int> QoalaHostQubitLifeTime::getLifeTimes() const {
        return this->qubitLifeTimes;
    }

} // namespace qoala::analysis::qbitlife
