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
            OP_BGT,
            OP_BLT,
            OP_LOAD,
            OP_STORE
        };
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
    };
}

#endif //QOALA_MLIR_ASM_H
