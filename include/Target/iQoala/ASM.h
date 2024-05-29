#ifndef QOALA_MLIR_ASM_H
#define QOALA_MLIR_ASM_H

#include "llvm/Support/raw_ostream.h"
#include "Target/iQoala/MC/iQoalaMC.h"

using namespace llvm;

namespace qoala::assembly {
    class NetQASMInstruction : public iQoalaMCInstruction {
    public:
        enum OpCode {
            OP_UNKNOWN = 0,
            OP_SET,
            OP_ADD,
            OP_SUBTRACT,
            OP_MULTIPLY,
            OP_QUOT,
            OP_REM,
            OP_JUMP,
            OP_BEQ,
            OP_BNE,
            OP_BGE,
            OP_BLE,
            OP_LOAD,
            OP_STORE
        };

        void print(raw_ostream &os) const override;
        virtual void printInstrInGenericForm(const std::string &mnemonic, raw_ostream &os) const = 0;
        virtual void printStoreOrLoad(raw_ostream &os) const = 0;
    };

    class QoalaHostInstruction : public iQoalaMCInstruction {
    public:
        enum OpCode {
            OP_UNKNOWN = 0,
            OP_ASSIGN,
            OP_ADD,
            OP_SUBTRACT,
            OP_MULTIPLY_CONSTANT,
            OP_BI_COND_MULTIPLY,
            OP_JUMP,
            OP_BEQ,
            OP_BNE,
            OP_BGT,
            OP_BLT,
            OP_SEND_MSG,
            OP_RECV_MSG,
            OP_RUN_ROUTINE,
            OP_RUN_REQUEST,
            OP_SUBMIT_ROUTINES,
            OP_JOIN_ROUTINES
        };

        void print(raw_ostream &os) const override;
        virtual void printInstruction(std::string &mnemonic, raw_ostream &os) const = 0;
    };

    raw_ostream &operator<<(raw_ostream &os, const iQoalaMCInstruction &instr);
    raw_ostream &operator<<(raw_ostream &os, const iQoalaMCOperand &oper);
    raw_ostream &operator<<(raw_ostream &os, const iQoalaExpr &expr);
}

#endif //QOALA_MLIR_ASM_H
