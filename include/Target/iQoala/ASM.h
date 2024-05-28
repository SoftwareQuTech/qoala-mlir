#ifndef QOALA_MLIR_ASM_H
#define QOALA_MLIR_ASM_H

#include "llvm/Support/raw_ostream.h"
#include "Target/iQoala/MC/iQoalaMC.h"

using namespace llvm;

namespace qoala::assembly {
    class NetQASMInstruction : public iQoalaMCInstruction {
    public:
        void print(raw_ostream &os) const override {};
        // TODO
    };

    class QoalaHostInstruction : public iQoalaMCInstruction {
    public:
        void print(raw_ostream &os) const override {};
        // TODO
    };
}

#endif //QOALA_MLIR_ASM_H
