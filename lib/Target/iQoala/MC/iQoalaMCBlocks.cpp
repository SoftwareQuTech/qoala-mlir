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

    // TODO - Move this constant to the iQoalaContext object
    unsigned int blockNumber = 0;

    void Block::print(raw_ostream &os) const {
        os << "b" << blockNumber << " { type = " << this->type << " }\n";
        for (const assembly::QoalaHostMCInstr *instruction : this->instructions) {
            os << tabStr << *instruction << "\n";
        }
    }
}