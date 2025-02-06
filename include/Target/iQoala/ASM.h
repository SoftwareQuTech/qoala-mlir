#ifndef QOALA_MLIR_ASM_H
#define QOALA_MLIR_ASM_H

#include "llvm/Support/raw_ostream.h"
#include "Analysis/Helpers/Helpers.h"

using namespace llvm;
using namespace qoala::helpers;

namespace qoala::assembly {
    class ASMInstruction : public PrintInterface { };

    class NetQASMInstruction : public ASMInstruction {
    public:
        void print(raw_ostream &os) const override {};
        // TODO
    };

    class QoalaHostInstruction : public ASMInstruction {
    public:
        void print(raw_ostream &os) const override {};
        // TODO
    };
}

#endif //QOALA_MLIR_ASM_H
