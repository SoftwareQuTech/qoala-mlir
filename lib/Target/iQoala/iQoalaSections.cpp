#include "Target/iQoala/iQoala.h"
#include "Analysis/Helpers/Helpers.h"

namespace qoala::iqoala {
    void MetaSection::print(raw_ostream &os) const {
        os << "META START\n";
        os << tabStr << "name: " << this->name << "\n";
        os << tabStr << "parameters: " << qoala::helpers::formatVector(this->globalParams) << "\n";
        os << tabStr << "csockets: " << qoala::helpers::formatMap(this->classicalSocketsMap) << "\n";
        os << tabStr << "epr_sockets: " << qoala::helpers::formatMap(this->eprsSocketsMap) << "\n";
        os << "META END\n";
    }

    void HostSection::print(raw_ostream &os) const {
        // Iteratively print all the blocks
        for (const Block &block : this->hostBlocks) {
            os << block << "\n";
        }
    }

    void NetQASMSection::print(raw_ostream &os) const {
        // Iteratively print all the routines:
        for (const QuantumRoutine &routine : this->routines) {
            os << routine << "\n";
        }
    }

    void RequestSection::print(llvm::raw_ostream &os) const {
        // Iteratively print all the routines:
        for (const QuantumRoutine &routine : this->routines) {
            os << routine << "\n";
        }
    }
}