#ifndef QOALA_MLIR_ASM_H
#define QOALA_MLIR_ASM_H

#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace qoala {
    struct PrintInterface {
    public:
        PrintInterface() = default;
        virtual ~PrintInterface() = default;
        virtual void print(raw_ostream &os) const = 0;
    };
}

namespace qoala::assembly {
    class ASMInstruction : public qoala::PrintInterface { };

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
