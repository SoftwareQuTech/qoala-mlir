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
        enum ExprKind {
            INVALID,
            SYMBOL_REFERENCE,
            CONSTANT_I32,
            CONSTANT_F32
        };
        ExprKind kind;
        union {
            char *symbolName;
            uint32_t i32ConstVal;
            float f32ConstVal;
        };

        iQoalaExpr() : kind(INVALID), i32ConstVal(0) { };

        bool isValid() const;
        bool isSymbolRef() const;
        bool isConstant() const;
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
            expr.kind = CONSTANT_I32;
            expr.i32ConstVal = value;
            return expr;
        }

        static iQoalaExpr createConstant(float value) {
            iQoalaExpr expr;
            expr.kind = CONSTANT_F32;
            expr.f32ConstVal = value;
            return expr;
        }
    };

    class iQoalaMCOperand : PrintInterface {
    public:
        struct iQoalaRegReference {
            iQoalaRegType type;
            uint32_t num;
        public :
            iQoalaRegReference() : type(iQoalaRegType::R), num(0) { }
            iQoalaRegReference(iQoalaRegType type, uint32_t num) : type(type), num(num) { }
            ~iQoalaRegReference() = default;
        };

        enum OperandKind {
            INVALID,
            IMMEDIATE_I32,
            IMMEDIATE_F32,
            REGISTER,
            LOCAL_REGISTER,
            EXPRESSION
        };

        OperandKind kind;
        union {
            uint32_t integerVal;
            float floatingPointVal;
            iQoalaRegReference *regRef;
            iQoalaExpr *expression;
            uint32_t localRegNum;
        };

        iQoalaMCOperand() : kind(INVALID), integerVal(0) { };
        ~iQoalaMCOperand() override { };
        bool isValid() const;
        bool isImmediate() const;
        bool isRegister() const;
        bool isLocalRegister() const;
        bool isExpression() const;
    public:
        void print(raw_ostream &os) const override;

        static iQoalaMCOperand createImmediate(uint32_t val) {
            iQoalaMCOperand operand;
            operand.kind = IMMEDIATE_I32;
            operand.integerVal = val;
            return operand;
        }

        static iQoalaMCOperand createImmediate(float val) {
            iQoalaMCOperand operand;
            operand.kind = IMMEDIATE_F32;
            operand.floatingPointVal = val;
            return operand;
        }

        static iQoalaMCOperand createRegister(iQoalaRegReference *regRef) {
            iQoalaMCOperand operand;
            operand.kind = REGISTER;
            operand.regRef = regRef;
            return operand;
        }

        static iQoalaMCOperand createExpr(iQoalaExpr *expr) {
            iQoalaMCOperand operand;
            operand.kind = EXPRESSION;
            operand.expression = expr;
            return operand;
        }
    };

    class iQoalaMCInstruction : public PrintInterface {
    public:
        /* The first declaration of all op codes is assumed to mean "unknown" */
        iQoalaMCInstruction(Operation *op) : originalOp(op), opCode(0) { }
        iQoalaMCInstruction(Operation *op, const uint32_t opCode) : originalOp(op), opCode(opCode), operands({}) { }
        iQoalaMCInstruction(const iQoalaMCInstruction &inst) : originalOp(inst.originalOp), opCode(inst.opCode), operands(inst.operands) { }
        ~iQoalaMCInstruction() override = default;

        void setOpcode(unsigned int opCode);
        unsigned int getOpcode() const;

        const iQoalaMCOperand &getOperand(unsigned i) const;
        iQoalaMCOperand &getOperand(unsigned i);
        unsigned int getNumOperands() const;

        void addOperand(const iQoalaMCOperand &op);
    protected:
        Operation *originalOp;
        unsigned int opCode;
        std::vector<iQoalaMCOperand> operands;
    };
}

#endif //QOALA_MLIR_IQOALAMC_H
