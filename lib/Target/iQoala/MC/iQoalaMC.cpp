#include "Target/iQoala/MC/iQoalaMC.h"

namespace qoala::assembly {
    /* General functions for the ASM classes */
    bool iQoalaExpr::isValid() const { return kind != INVALID; }
    bool iQoalaExpr::isSymbolRef() const { return kind == SYMBOL_REFERENCE; }
    bool iQoalaExpr::isConstant() const { return kind == CONSTANT_I32 || kind == CONSTANT_F32; }

    bool iQoalaMCOperand::isValid() const { return kind != INVALID; }
    bool iQoalaMCOperand::isImmediate() const { return kind == IMMEDIATE_I32 || kind == IMMEDIATE_F32; }
    bool iQoalaMCOperand::isRegister() const { return kind == REGISTER; }
    bool iQoalaMCOperand::isLocalRegister() const { return kind == LOCAL_REGISTER; }
    bool iQoalaMCOperand::isExpression() const { return kind == EXPRESSION; }

    void iQoalaMCInstruction::setOpcode(unsigned int newOpCode) { this->opCode = newOpCode; }
    unsigned int iQoalaMCInstruction::getOpcode() const { return opCode; }

    const iQoalaMCOperand &iQoalaMCInstruction::getOperand(unsigned i) const { return operands[i]; }
    iQoalaMCOperand &iQoalaMCInstruction::getOperand(unsigned i) { return operands[i]; }
    unsigned int iQoalaMCInstruction::getNumOperands() const { return operands.size(); }

    void iQoalaMCInstruction::addOperand(const iQoalaMCOperand &op) { operands.push_back(op); }

    void iQoalaExpr::print(mlir::raw_ostream &os) const {
        switch(this->kind) {
            case INVALID:
                assert(false && "Expression is of invalid kind.\n");
            case SYMBOL_REFERENCE:
                os << this->symbolName;
                break;
            case CONSTANT_I32:
                os << this->i32ConstVal;
                break;
            case CONSTANT_F32:
                os << this->f32ConstVal;
                break;
        }
    }

    static std::string formatRegister(const iQoalaMCOperand::iQoalaRegReference *registerRef) {
        std::stringstream formattedRegStr;
        switch (registerRef->type) {
            case R:
                formattedRegStr << "R" << registerRef->num;
                break;
            case C:
                formattedRegStr << "C" << registerRef->num;
                break;
            case M:
                formattedRegStr << "M" << registerRef->num;
                break;
            case Q:
                formattedRegStr << "Q" << registerRef->num;
                break;
        }
        return formattedRegStr.str();
    }

    void iQoalaMCOperand::print(llvm::raw_ostream &os) const {
        switch(this->kind) {
            case INVALID:
                assert(false && "Op code for operand is unknown.\n");
            case IMMEDIATE_I32:
                os << this->integerVal;
                break;
            case IMMEDIATE_F32:
                os << this->floatingPointVal;
                break;
            case REGISTER:
                os << formatRegister(this->regRef);
                break;
            case LOCAL_REGISTER:
                os << "%" << this->localRegNum;
                break;
            case EXPRESSION:
                os << this->expression;
                break;
        }
    }
}