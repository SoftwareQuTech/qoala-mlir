#include "Target/iQoala/iQoala.h"
#include "Analysis/Helpers/Helpers.h"

namespace qoala::iqoala {
    void LocalQuantumRoutine::print(raw_ostream &os) const {
        os << "SUBROUTINE " << this->name << "\n";
        os << "params:" << qoala::helpers::formatVector(this->params) << "\n";
        os << "returns:" << qoala::helpers::formatVector(this->returns) << "\n";
        os << "uses:" << qoala::helpers::formatVector(this->usesQubits) << "\n";
        os << "keeps:" << qoala::helpers::formatVector(this->keepsQubits) << "\n";

        os << "NETQASM_START\n";
        for (const NetQASMInstruction &instruction : this->instructions) {
            os << tabStr << instruction << "\n";
        }
        os << "NETQASM_END\n";
    }

    void RequestQuantumRoutine::print(raw_ostream &os) const {
        // TODO
    }
}