#ifndef QOALA_MLIR_ASM_H
#define QOALA_MLIR_ASM_H

#include "llvm/Support/raw_ostream.h"
#include "Target/iQoala/MC/iQoalaMC.h"

namespace qoala::assembly {
    class NetQASMBaseInstr : public iQoalaMCInstruction {
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

        NetQASMBaseInstr();

        void print(mlir::raw_ostream &os) const override;
    private:
        void printInstrInGenericForm(const std::string &mnemonic, mlir::raw_ostream &os) const;
        void printStoreOrLoad(mlir::raw_ostream &os) const;
    };

    class QoalaHostInstr : public iQoalaMCInstruction {
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

        QoalaHostInstr();

        void print(mlir::raw_ostream &os) const override;
    private:
        void printInstrGeneric(const std::string &mnemonic, mlir::raw_ostream &os,
                                       const iQoalaMCOperand *ssaLocalReg = nullptr,
                                       const iQoalaMCOperand *immediateExpr = nullptr) const;
    };

    mlir::raw_ostream &operator<<(mlir::raw_ostream &os, const iQoalaMCInstruction &instr);
    mlir::raw_ostream &operator<<(mlir::raw_ostream &os, const iQoalaMCOperand &oper);
    mlir::raw_ostream &operator<<(mlir::raw_ostream &os, const iQoalaExpr &expr);
}

#endif //QOALA_MLIR_ASM_H
