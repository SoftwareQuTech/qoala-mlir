#include "Target/iQoala/iQoala.h"

namespace qoala::iqoala {
    raw_ostream &operator<<(raw_ostream &os, const BlockType &block) {
        switch (block.type) {
            case BlockType::BlockTypeTy::CC:
                os << "CC";
                break;
            case BlockType::BlockTypeTy::CL:
                os << "CL";
                break;
            case BlockType::BlockTypeTy::QC:
                os << "QC";
                break;
            case BlockType::BlockTypeTy::QL:
                os << "QL";
                break;
        }
        return os;
    }

    unsigned int blockNumber = 0;

    void Block::print(raw_ostream &os) const {
        os << "b" << blockNumber << " { type = " << this->type << " }\n";
        for (const QoalaHostInstruction &instruction : this->instructions) {
            os << tabStr << instruction << "\n";
        }
    }
}