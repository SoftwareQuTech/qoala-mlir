#include "Analysis/QoalaHost/Helpers.h"

using namespace qoala::analysis::reordering;

// === MILPOperation ===
MILPOperation::MILPOperation(const std::string &id, OpType type, double duration) :
    id_(id), type_(type), duration_(duration), start_time_(0.0) {}

const std::string &MILPOperation::getId() const { return id_; }
OpType MILPOperation::getType() const { return type_; }
double MILPOperation::getDuration() const { return duration_; }
void MILPOperation::setStartTime(double startTime) { start_time_ = startTime; }
double MILPOperation::getStartTime() const { return start_time_; }

// === MILPTask ===
MILPTask::MILPTask(int id, MILPBlock *parent, const std::string &group) :
    id_(id), parent_block_(parent), group_(group) {}

int MILPTask::getId() const { return id_; }
const std::string &MILPTask::getGroup() const { return group_; }
void MILPTask::addOperation(MILPOperation *op) { operations_.push_back(op); }
const std::vector<MILPOperation *> &MILPTask::getOperations() const { return operations_; }
MILPOperation *MILPTask::getFirstOperation() const { return operations_.empty() ? nullptr : operations_.front(); }
double MILPTask::getDuration() const {
    double dur = 0.0;
    for (auto *op: operations_)
        dur += op->getDuration();
    return dur;
}
MILPBlock *MILPTask::getParentBlock() const { return parent_block_; }

// === MILPBlock ===
MILPBlock::MILPBlock(const std::string &id, OpType type) : id_(id), type_(type) {}

const std::string &MILPBlock::getId() const { return id_; }
OpType MILPBlock::getType() const { return type_; }
void MILPBlock::addOperation(MILPOperation *op) { operations_.push_back(op); }
const std::vector<MILPOperation *> &MILPBlock::getOperations() const { return operations_; }
void MILPBlock::addTask(MILPTask *task) { tasks_.push_back(task); }
const std::vector<MILPTask *> &MILPBlock::getTasks() const { return tasks_; }

// === MILPQubit ===
MILPQubit::MILPQubit(const std::string &id) : id_(id), alloc_op_(nullptr), meas_op_(nullptr) {}

const std::string &MILPQubit::getId() const { return id_; }
void MILPQubit::setAllocation(MILPOperation *allocOp) { alloc_op_ = allocOp; }
void MILPQubit::setMeasurement(MILPOperation *measOp) { meas_op_ = measOp; }
MILPOperation *MILPQubit::getAllocation() const { return alloc_op_; }
MILPOperation *MILPQubit::getMeasurement() const { return meas_op_; }

double MILPQubit::computeLifetime() const {
    if (!alloc_op_ || !meas_op_)
        return 0.0;
    return (meas_op_->getStartTime() + meas_op_->getDuration()) -
           (alloc_op_->getStartTime() + alloc_op_->getDuration());
}
