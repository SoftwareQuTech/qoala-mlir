#include "Target/iQoala/iQoala.h"

namespace qoala::iqoala {

    raw_ostream &operator<<(raw_ostream &os, const PrintInterface &printable) {
        printable.print(os);
        return os;
    }

    void iQoalaProgram::print(raw_ostream &os) const {
        // Call the "print" functions in the sections of the executable
        os << this->metaSection << "\n" << this->hostSection << "\n" << this->netQASMSection << "\n";
    }
}
