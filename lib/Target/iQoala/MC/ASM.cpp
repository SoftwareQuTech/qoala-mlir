#include "Target/iQoala/MC/ASM.h"
#include <iterator>

using namespace mlir;

namespace qoala::assembly {
    // Implementations of the "<<" operator
    raw_ostream &operator<<(raw_ostream &os, const iQoalaMCInstruction &instr) {
        instr.print(os);
        return os;
    }
    raw_ostream &operator<<(raw_ostream &os, const iQoalaMCOperand &oper) {
        oper.print(os);
        return os;

    }
    raw_ostream &operator<<(raw_ostream &os, const iQoalaExpr &expr) {
        expr.print(os);
        return os;

    }
}