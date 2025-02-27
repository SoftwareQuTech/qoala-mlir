#include "Target/iQoala/MC/iQoalaMC.h"
#include "llvm/Support/raw_ostream.h"
#include "mlir/IR/Diagnostics.h"

namespace qoala::assembly {
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


    const iQoalaMCOperand &iQoalaMCInstruction::getOperand(unsigned i) const { return operands[i]; }
    iQoalaMCOperand &iQoalaMCInstruction::getOperand(unsigned i) { return operands[i]; }
    unsigned int iQoalaMCInstruction::getNumOperands() const { return operands.size(); }

    void iQoalaMCInstruction::addOperand(iQoalaMCOperand &op) {
        op.setInst(this);
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
                this->inst->getOriginalOp()->emitError("Float immediate is not supported yet!");
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