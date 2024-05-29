#ifndef QOALA_MLIR_IQOALAMC_H
#define QOALA_MLIR_IQOALAMC_H

#include "Analysis/Helpers/Helpers.h"
#include "llvm/ADT/SmallVector.h"

// This file defines iQoalaMCOperand and iQoalaMCInst, which represent the
// low-level "machine code" instructions and operands.
// NetQASM, QoalaHost and QRemote instructions will be translated into these
// objects, so they can be easily printed
// These definitions are heavily inspired in the LLVM's definitions located
// in "include/llvm/MC/MCInst.h", but this is a way lighter model than the one
// presented in LLVM

using namespace qoala::helpers;
namespace qoala::assembly {
    enum iQoalaRegType { R, C, M, Q };

    class iQoalaExpr : PrintInterface {
        enum iQoalaExprType {
            INVALID,
            SYMBOL_REFERENCE,
            CONSTANT
        };
        iQoalaExprType kind;
        union {
            char *symbolName;
            uint32_t i32ConstVal;
            float f32ConstVal;
        };

        iQoalaExpr() : kind(INVALID), i32ConstVal(0) { };

        bool isValid() { return kind != INVALID; }
        bool isSymbolRef() { return kind == SYMBOL_REFERENCE; }
        bool isConstant() { return kind == CONSTANT; }
    public:
        void print(raw_ostream &os) const override;

        static iQoalaExpr createSymbolRef(std::string symName) {
            iQoalaExpr expr;
            expr.kind = SYMBOL_REFERENCE;
            expr.symbolName = symName.data();
            return expr;
        }

        static iQoalaExpr createConstant(uint32_t value) {
            iQoalaExpr expr;
            expr.kind = CONSTANT;
            expr.i32ConstVal = value;
            return expr;
        }

        static iQoalaExpr createConstant(float value) {
            iQoalaExpr expr;
            expr.kind = CONSTANT;
            expr.f32ConstVal = value;
            return expr;
        }
    };

    class iQoalaMCOperand : PrintInterface {
    private:
        struct iQoalaRegReference {
            iQoalaRegType type;
            uint32_t num;
        };
    public:
        enum iQoalaMCOperandType {
            INVALID,
            IMMEDIATE,
            REGISTER,
            EXPRESSION
        };
        iQoalaMCOperandType kind;
        union {
            uint32_t integerVal;
            float floatingPointVal;
            iQoalaRegReference regRef;
        };

        iQoalaMCOperand() : kind(INVALID), integerVal(0) { };
        bool isValid() const { return kind != INVALID; }
        bool isImmediate() const { return kind == IMMEDIATE; }
        bool isRegister() const { return kind == REGISTER; }
    public:
        void print(raw_ostream &os) const override;

        static iQoalaMCOperand createImmediate(uint32_t val) {
            iQoalaMCOperand operand;
            operand.kind = IMMEDIATE;
            operand.integerVal = val;
            return operand;
        }

        static iQoalaMCOperand createImmediate(float val) {
            iQoalaMCOperand operand;
            operand.kind = IMMEDIATE;
            operand.floatingPointVal = val;
            return operand;
        }

        static iQoalaMCOperand createRegister(iQoalaRegType regType, uint32_t regNum) {
            iQoalaMCOperand operand;
            operand.kind = REGISTER;
            operand.regRef = {regType, regNum};
            return operand;
        }
    };

    class iQoalaMCInstruction : public PrintInterface {
    public:
        iQoalaMCInstruction() = default;

        void setOpcode(unsigned int opCode) { this->opCode = opCode; }
        unsigned int getOpcode() const { return opCode; }

        const iQoalaMCOperand &getOperand(unsigned i) const { return operands[i]; }
        iQoalaMCOperand &getOperand(unsigned i) { return operands[i]; }
        unsigned int getNumOperands() const { return operands.size(); }

        void addOperand(const iQoalaMCOperand op) { operands.push_back(op); }
    protected:
        Operation *originalOp;
        unsigned int opCode = 0; /*the first declaration of all op codes is assumed to mean "unknown" */
        llvm::SmallVector<iQoalaMCOperand, 10> operands;
    };
}

#endif //QOALA_MLIR_IQOALAMC_H
