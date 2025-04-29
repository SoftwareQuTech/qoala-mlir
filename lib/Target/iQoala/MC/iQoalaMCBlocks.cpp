#include "Target/iQoala/iQoala.h"

using namespace mlir;

namespace qoala::iqoala {
    void Block::appendInstruction(assembly::QoalaHostMCInstr *instruction) {
        this->instructions.push_back(instruction);
    }

    bool Block::blockContainsRunRequest() const {
        return std::any_of(this->instructions.begin(), this->instructions.end(),
                           [](const assembly::QoalaHostMCInstr *instruction) {
                               if (instruction->getOpcode() == assembly::QoalaHostMCInstr::OpCode::OP_RUN_REQUEST) {
                                   return true;
                               }
                               return false;
                           });
    }

    bool Block::blockContainsRunSubRoutine() const {
        return std::any_of(this->instructions.begin(), this->instructions.end(),
                           [](const assembly::QoalaHostMCInstr *instruction) {
                               if (instruction->getOpcode() == assembly::QoalaHostMCInstr::OpCode::OP_RUN_SUBROUTINE) {
                                   return true;
                               }
                               return false;
                           });
    }

    bool Block::blockContainsRecvMsg() const {
        return std::any_of(this->instructions.begin(), this->instructions.end(),
                           [](const assembly::QoalaHostMCInstr *instruction) {
                               if (instruction->getOpcode() == assembly::QoalaHostMCInstr::OpCode::OP_RECV_MSG) {
                                   return true;
                               }
                               return false;
                           });
    }

    raw_ostream &operator<<(raw_ostream &os, const Block::BlockType block) {
        switch (block) {
            case Block::CC:
                os << "CC";
                break;
            case Block::CL:
                os << "CL";
                break;
            case Block::QC:
                os << "QC";
                break;
            case Block::QL:
                os << "QL";
                break;
        }
        return os;
    }

    void Block::print(raw_ostream &os) const {
        std::vector<std::string> predNames;
        for (auto pred: this->predecessors) {
            predNames.push_back(pred->name);
        }
        os << "^" << this->name << " { type = " << this->type << ", predecessors = [" << helpers::formatVector(predNames)
           << "] }:\n";
        for (const assembly::QoalaHostMCInstr *instruction: this->instructions) {
            os << tabStr << *instruction << "\n";
        }
    }
} // namespace qoala::iqoala
