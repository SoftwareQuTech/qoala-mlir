#include "Target/iQoala/MC/iQoalaMC.h"
#include "llvm/Support/raw_ostream.h"
#include "mlir/IR/Diagnostics.h"

namespace qoala::assembly {
    /* Builders */
    iQoalaRegReference *iQoalaRegReference::createRegReference(const iQoalaRegType type, const uint32_t num) {
        const auto regReference = new iQoalaRegReference();
        regReference->type = type;
        regReference->num = num;
        return regReference;
    }

    iQoalaMCExpr *iQoalaMCExpr::createSymbolRef(std::string symName) {
        const auto expr = new iQoalaMCExpr();
        expr->kind = SYMBOL_REFERENCE;
        expr->symbolName = symName.data();
        return expr;
    }

    iQoalaMCExpr *iQoalaMCExpr::createConstant(const uint32_t value) {
        const auto expr = new iQoalaMCExpr();
        expr->kind = CONSTANT_I32;
        expr->i32ConstVal = value;
        return expr;
    }

    iQoalaMCExpr *iQoalaMCExpr::createConstant(const float value) {
        const auto expr = new iQoalaMCExpr();
        expr->kind = CONSTANT_F32;
        expr->f32ConstVal = value;
        return expr;
    }

    iQoalaMCOperand *iQoalaMCOperand::createImmediateOperand(const uint32_t val) {
        const auto operand = new iQoalaMCOperand();
        operand->kind = IMMEDIATE_I32;
        operand->integerVal = val;
        return operand;
    }

    iQoalaMCOperand *iQoalaMCOperand::createImmediateOperand(const float val) {
        const auto operand = new iQoalaMCOperand();
        operand->kind = IMMEDIATE_F32;
        operand->floatingPointVal = val;
        return operand;
    }

    iQoalaMCOperand *iQoalaMCOperand::createRegisterOperand(iQoalaRegReference *regRef) {
        const auto operand = new iQoalaMCOperand();
        switch (regRef->getType()) {
            case LOCAL:
                operand->kind = LOCAL_REGISTER;
                break;
            case R:
            case C:
            case M:
            case Q:
                operand->kind = REGISTER;
                break;
        }
        operand->regRef = regRef;
        return operand;
    }

    iQoalaMCOperand *iQoalaMCOperand::createExprOperand(iQoalaMCExpr *expr) {
        const auto operand = new iQoalaMCOperand();
        operand->kind = EXPRESSION;
        operand->expression = expr;
        return operand;
    }

    iQoalaMCExpr *iQoalaMCExpr::createSymbolRefExpr(const std::string &symbolName) {
        return new iQoalaMCExpr(symbolName);
    }

    uint32_t iQoalaMCOperand::getIntegerVal() const {
        assert(this->isImmediate());
        return this->integerVal;
    }

    float iQoalaMCOperand::getFloatingPointVal() const {
        assert(this->isImmediate());
        return this->floatingPointVal;
    }
    iQoalaRegReference *iQoalaMCOperand::getRegRef() const {
        assert(this->isLocalRegister() || this->isRegister());
        return this->regRef;
    }

    iQoalaMCExpr *iQoalaMCOperand::getExpression() const {
        assert(this->isExpression());
        return this->expression;
    }

    /* General functions for the ASM classes */
    bool iQoalaMCExpr::isValid() const { return kind != INVALID; }
    bool iQoalaMCExpr::isSymbolRef() const { return kind == SYMBOL_REFERENCE; }
    bool iQoalaMCExpr::isConstant() const { return kind == CONSTANT_I32 || kind == CONSTANT_F32; }

    void iQoalaMCOperand::setInst(iQoalaMCInstruction *inst) { this->inst = inst; }

    bool iQoalaMCOperand::isValid() const { return kind != INVALID; }
    bool iQoalaMCOperand::isImmediate() const { return kind == IMMEDIATE_I32 || kind == IMMEDIATE_F32; }
    bool iQoalaMCOperand::isRegister() const { return kind == REGISTER; }
    bool iQoalaMCOperand::isLocalRegister() const { return kind == LOCAL_REGISTER; }
    bool iQoalaMCOperand::isExpression() const { return kind == EXPRESSION; }

    void iQoalaMCInstruction::setOpcode(unsigned int newOpCode) { this->opCode = newOpCode; }
    unsigned int iQoalaMCInstruction::getOpcode() const { return opCode; }
    mlir::Operation *iQoalaMCInstruction::getOriginalOp() const { return this->originalOp; }

    iQoalaMCOperand *iQoalaMCInstruction::getOperand(unsigned i) const { return operands[i]; }
    iQoalaMCOperand *iQoalaMCInstruction::getOperand(unsigned i) { return operands[i]; }
    unsigned int iQoalaMCInstruction::getNumOperands() const { return operands.size(); }

    void iQoalaMCInstruction::addOperand(iQoalaMCOperand *op) {
        op->setInst(this);
        operands.push_back(op);
    }

    void iQoalaMCExpr::print(mlir::raw_ostream &os) const {
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
                mlir::emitError(mlir::UnknownLoc()) << "Floats are not supported yet.";
                os << this->f32ConstVal;
                break;
        }
    }

    std::string iQoalaRegReference::formatRegister() const {
        std::stringstream formattedRegStr;
        switch (this->type) {
            case LOCAL:
                formattedRegStr << "%" << this->num;
                break;
            case R:
                formattedRegStr << "R" << this->num;
                break;
            case C:
                formattedRegStr << "C" << this->num;
                break;
            case M:
                formattedRegStr << "M" << this->num;
                break;
            case Q:
                formattedRegStr << "Q" << this->num;
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
                this->inst->getOriginalOp()->emitError("Float immediate is not supported yet!");
                os << this->floatingPointVal;
                break;
            case LOCAL_REGISTER:
            case REGISTER:
                os << this->regRef->formatRegister();
                break;
            case EXPRESSION:
                os << this->expression;
                break;
        }
    }

    // Implementations of the "<<" operator
    mlir::raw_ostream &operator<<(mlir::raw_ostream &os, const iQoalaMCInstruction &instr) {
        instr.print(os);
        return os;
    }
    mlir::raw_ostream &operator<<(mlir::raw_ostream &os, const iQoalaMCOperand &oper) {
        oper.print(os);
        return os;

    }
    mlir::raw_ostream &operator<<(mlir::raw_ostream &os, const iQoalaMCExpr &expr) {
        expr.print(os);
        return os;

    }
}