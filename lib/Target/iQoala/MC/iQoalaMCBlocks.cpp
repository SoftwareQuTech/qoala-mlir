#include "Target/iQoala/iQoala.h"

using namespace mlir;

namespace qoala::iqoala {
    void Block::appendInstruction(assembly::QoalaHostMCInstr *instruction) {
        this->instructions.push_back(instruction);
    }

    raw_ostream &operator<<(raw_ostream &os, Block::BlockType block) {
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
        os << this->name << " { type = " << this->type << " }:\n";
        for (const assembly::QoalaHostMCInstr *instruction : this->instructions) {
            os << tabStr << *instruction << "\n";
        }
    }
}