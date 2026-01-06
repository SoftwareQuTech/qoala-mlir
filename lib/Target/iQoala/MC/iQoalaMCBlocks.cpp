#include "Target/iQoala/iQoala.h"

using namespace mlir;

namespace qoala::iqoala {
    void Block::appendInstruction(assembly::QoalaHostMCInstr *instruction) {
        this->instructions.push_back(instruction);
    }

    void Block::removePredecessorIfExists(const Block *pred) {
        auto position = std::find(this->predecessors.begin(), this->predecessors.end(), pred);
        if (position != this->predecessors.end()) {
            this->predecessors.erase(position);
        }
    }

    void Block::removeDependencyIfExists(const Block *dep) {
        auto position = std::find(this->dependencies.begin(), this->dependencies.end(), dep);
        if (position != this->dependencies.end()) {
            this->dependencies.erase(position);
        }
    }

    void Block::removePrevCommIfExists(const Block *oldPrevComm) {
        if (this->prevComm == oldPrevComm) {
            this->prevComm = nullptr;
        }
    }

    void Block::removePrevEntIfExists(const Block *oldPrevEnt) {
        if (this->prevEnt == oldPrevEnt) {
            this->prevEnt = nullptr;
        }
    }

    void Block::removeAllBlockReferences(const std::set<Block *> &blockPtrs) {
        for (const Block *blockPtr : blockPtrs) {
            this->removeDependencyIfExists(blockPtr);
            this->removePredecessorIfExists(blockPtr);
            this->removePrevCommIfExists(blockPtr);
            this->removePrevEntIfExists(blockPtr);
        }
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
                               if (instruction->getOpcode() == assembly::QoalaHostMCInstr::OpCode::OP_RECV_CMSG) {
                                   return true;
                               }
                               return false;
                           });
    }

    bool Block::containsJumpTo(const Block *destination) {
        return std::any_of(this->instructions.begin(), this->instructions.end(),
                           [&](const assembly::QoalaHostMCInstr *instruction) {
                               if (instruction->getOpcode() == assembly::QoalaHostMCInstr::OpCode::OP_JUMP) {
                                   const assembly::iQoalaMCOperand *blockDst = instruction->getOperand(0);
                                   assert(blockDst->isExpression());
                                   const assembly::iQoalaMCExpr *blockDstExpression = blockDst->getExpression();
                                   assert(blockDstExpression->isSymbolRef());
                                   const std::string referencedBlock = blockDstExpression->getSymbolName();
                                   return destination->getName() == referencedBlock;
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
        for (auto pred : this->predecessors) {
            predNames.push_back(pred->name);
        }

        std::vector<std::string> depNames;
        for (auto dep : this->dependencies) {
            depNames.push_back(dep->name);
        }

        Block *prevCommBlk = this->prevComm;
        std::string prevComm;
        if (prevCommBlk != nullptr) {
            prevComm = prevCommBlk->name;
        }

        Block *prevEntBlk = this->prevEnt;
        std::string prevEnt;
        if (prevEntBlk != nullptr) {
            prevEnt = prevEntBlk->name;
        }

        std::unordered_map<std::string, int> deadlines;
        for (const auto &pair : this->deadlines) {
            deadlines[pair.first->name] = pair.second;
        }

        os << "^" << this->name << " { type = " << this->type << "; predecessors = ["
           << helpers::formatVector(predNames) << "]; dependencies = [" << helpers::formatVector(depNames)
           << "]; prev_comm = " << prevComm << "; prev_ent = " << prevEnt << "; deadlines = ["
           << helpers::formatUnorderedMap(deadlines) << "] }:\n";
        for (const assembly::QoalaHostMCInstr *instruction : this->instructions) {
            os << tabStr << *instruction << "\n";
        }
    }
} // namespace qoala::iqoala
