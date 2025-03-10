#include "Target/iQoala/MC/iQoalaMC.h"

using namespace mlir;

namespace qoala::assembly {
    static QoalaHostMCInstr *create3RegInstruction(
        Operation *op, const QoalaHostMCInstr::OpCode opCode,
        iQoalaMCOperand *reg0, iQoalaMCOperand *reg1, iQoalaMCOperand *reg2) {
        assert(reg0->isRegister() && "QoalaHost 3-reg instruction: operand 0 must be a register");
        assert(reg1->isRegister() && "QoalaHost 3-reg instruction: operand 1 must be a register");
        assert(reg2->isRegister() && "QoalaHost 3-reg instruction: operand 2 must be a register");
        const auto instruction = new QoalaHostMCInstr(op, opCode);
        instruction->addOperand(reg0);
        instruction->addOperand(reg1);
        instruction->addOperand(reg2);
        return instruction;
    }

    static QoalaHostMCInstr *create2Reg1ImmInstruction(
        Operation *op, const QoalaHostMCInstr::OpCode opCode,
        iQoalaMCOperand *reg0, iQoalaMCOperand *reg1, iQoalaMCOperand *imm) {
        assert(reg0->isRegister() && "QoalaHost 2-reg,1-imm instruction: operand 0 must be a register");
        assert(reg1->isRegister() && "QoalaHost 2-reg,1-imm instruction: operand 1 must be a register");
        assert(imm->isImmediate() && "QoalaHost 2-reg,1-imm instruction: operand 2 must be an immediate");
        const auto instruction = new QoalaHostMCInstr(op, opCode);
        instruction->addOperand(reg0);
        instruction->addOperand(reg1);
        instruction->addOperand(imm);
        return instruction;
    }

    static QoalaHostMCInstr *create2Reg1BlockRefInstruction(
        Operation *op, const QoalaHostMCInstr::OpCode opCode,
        iQoalaMCOperand *reg0, iQoalaMCOperand *reg1, iQoalaMCOperand *target) {
        assert(reg0->isRegister() && "QoalaHost 2-reg,1-block-ref instruction: operand 0 must be a register");
        assert(reg1->isRegister() && "QoalaHost 2-reg,1-block-ref instruction: operand 1 must be a register");
        assert(target->isExpression() && "QoalaHost 2-reg,1-block-ref instruction: operand 2 must be an expression");
        assert(target->getExpression()->isSymbolRef() && "QoalaHost 2-reg,1-block-ref instruction: operand 2 must be a block reference");
        const auto instruction = new QoalaHostMCInstr(op, opCode);
        instruction->addOperand(reg0);
        instruction->addOperand(reg1);
        instruction->addOperand(target);
        return instruction;
    }

    QoalaHostMCInstr *QoalaHostMCInstr::createAssignCValInstr(Operation *op, iQoalaMCOperand *reg, iQoalaMCOperand *imm) {
        assert(reg->isLocalRegister() && "first operand must be a local register");
        assert(imm->isImmediate() && "second operand must be an immediate");
        auto *instr = new QoalaHostMCInstr(op, OP_ASSIGN_CVAL);
        instr->addOperand(reg);
        instr->addOperand(imm);
        return instr;
    }

    QoalaHostMCInstr *QoalaHostMCInstr::createAddInstr(
        Operation *op, iQoalaMCOperand *resReg, iQoalaMCOperand *op1, iQoalaMCOperand *op2) {
        return create3RegInstruction(op, OP_ADD, resReg, op1, op2);
    }

    QoalaHostMCInstr *QoalaHostMCInstr::createSubInstr(
        Operation *op, iQoalaMCOperand *resReg, iQoalaMCOperand *op1, iQoalaMCOperand *op2) {
        return create3RegInstruction(op, OP_SUBTRACT, resReg, op1, op2);
    }

    QoalaHostMCInstr *QoalaHostMCInstr::createMulConstInstr(
        Operation *op, iQoalaMCOperand *resReg, iQoalaMCOperand *op1, iQoalaMCOperand *imm) {
        return create2Reg1ImmInstruction(op, OP_MULTIPLY_CONSTANT, resReg, op1, imm);
    }

    QoalaHostMCInstr *QoalaHostMCInstr::createRunRoutineInstruction(Operation *op) {
        auto *instr = new QoalaHostMCInstr(op, OP_RUN_ROUTINE);
        return instr;
    }

    QoalaHostMCInstr *QoalaHostMCInstr::createJmpInstr(Operation *op, iQoalaMCOperand *target) {
        assert(target->isExpression() && "QoalaHost jmp instruction: target operand must be an expression");
        assert(target->getExpression()->isSymbolRef() && "QoalaHost jmp instruction: target operand must be a block reference");
        auto *instr = new QoalaHostMCInstr(op, OP_JUMP);
        instr->addOperand(target);
        return instr;
    }

    QoalaHostMCInstr *QoalaHostMCInstr::createBeqInstr(
        Operation *op, iQoalaMCOperand *op1, iQoalaMCOperand *op2, iQoalaMCOperand *target) {
        return create2Reg1BlockRefInstruction(op, OP_BEQ, op1, op2, target);
    }

    QoalaHostMCInstr *QoalaHostMCInstr::createBneInstr(
        Operation *op, iQoalaMCOperand *op1, iQoalaMCOperand *op2, iQoalaMCOperand *target) {
        return create2Reg1BlockRefInstruction(op, OP_BNE, op1, op2, target);
    }

    QoalaHostMCInstr *QoalaHostMCInstr::createBgtInstr(
        Operation *op, iQoalaMCOperand *op1, iQoalaMCOperand *op2, iQoalaMCOperand *target) {
        return create2Reg1BlockRefInstruction(op, OP_BGT, op1, op2, target);
    }

    QoalaHostMCInstr *QoalaHostMCInstr::createBltInstr(
        Operation *op, iQoalaMCOperand *op1, iQoalaMCOperand *op2, iQoalaMCOperand *target) {
        return create2Reg1BlockRefInstruction(op, OP_BLT, op1, op2, target);
    }

    void QoalaHostMCInstr::printInstrGeneric(const std::string &mnemonic, raw_ostream &os,
                                           const iQoalaMCOperand *ssaLocalReg,
                                           const iQoalaMCOperand *immediateExpr) const {
        auto currentElement = this->operands.begin();
        auto nextElement = this->operands.begin();
        auto lastElement = this->operands.end();
        nextElement++;

        if (ssaLocalReg != nullptr) {
            os << *ssaLocalReg << " = ";
            currentElement++;
            nextElement++;
        }

        if (immediateExpr != nullptr) {
            currentElement++;
            nextElement++;
        }

        os << mnemonic << " (";

        for (; currentElement != this->operands.end(); currentElement++) {
            os << *currentElement << (nextElement != lastElement ? ", " : "");
        }
        os << ")";

        if (immediateExpr != nullptr) {
            // Print the immediate at last... if it was passed
            os << " : " << *immediateExpr;
        }
    }

    void QoalaHostMCInstr::print(raw_ostream &os) const {
        switch (this->opCode) {
            case OP_UNKNOWN:
                this->originalOp->emitError("Op code for operation '") << *this->originalOp << "' is unknown.\n";
                break;
            case OP_ASSIGN_CVAL:
                assert(this->operands.size() == 2);
                // We assume the first operand is the "name" of the local registry to assign to
                // And the second operand is the immediate
                assert(this->operands[0]->isLocalRegister());
                assert(this->operands[1]->isImmediate());
                printInstrGeneric("assign_cval", os, this->operands[0], this->operands[1]);
                break;
            case OP_ADD:
                assert(this->operands.size() == 3);
                // We assume the first operand is the "name" of the local registry to assign to
                assert(this->operands[0]->isLocalRegister());
                assert(this->operands[1]->isLocalRegister());
                assert(this->operands[2]->isLocalRegister());
                printInstrGeneric("add_cval_c", os, this->operands[0]);
                break;
            case OP_SUBTRACT:
                assert(this->operands.size() == 3);
                // We assume the first operand is the "name" of the local registry to assign to
                assert(this->operands[0]->isLocalRegister());
                assert(this->operands[1]->isLocalRegister());
                assert(this->operands[2]->isLocalRegister());
                printInstrGeneric("sub_cval_c", os, this->operands[0]);
                break;
            case OP_MULTIPLY_CONSTANT:
                assert(this->operands.size() == 3);
                // We assume the first operand is the "name" of the local registry to assign to
                // And the second operand is the immediate
                assert(this->operands[0]->isLocalRegister());
                assert(this->operands[1]->isImmediate());
                assert(this->operands[2]->isLocalRegister());
                printInstrGeneric("mult_const", os, this->operands[0], this->operands[1]);
                break;
            case OP_BI_COND_MULTIPLY:
                assert(this->operands.size() == 4);
                // We assume the first operand is the "name" of the local registry to assign to
                // And the second operand is the immediate
                assert(this->operands[0]->isLocalRegister());
                assert(this->operands[1]->isImmediate());
                assert(this->operands[2]->isLocalRegister());
                assert(this->operands[3]->isLocalRegister());
                printInstrGeneric("bcond_mult_const", os, this->operands[0], this->operands[1]);
                break;
            case OP_JUMP:
                assert(this->operands.size() == 1);
                assert(this->operands[0]->isExpression());
                printInstrGeneric("jump", os, nullptr, this->operands[0]);
                break;
            case OP_BEQ:
                // We assume the first operand is the symbol reference to jump to
                assert(this->operands.size() == 3);
                assert(this->operands[0]->isRegister());
                assert(this->operands[1]->isExpression());
                assert(this->operands[2]->isRegister());
                printInstrGeneric("beq", os, nullptr, this->operands[0]);
                break;
            case OP_BNE:
                // We assume the first operand is the symbol reference to jump to
                assert(this->operands.size() == 3);
                assert(this->operands[0]->isRegister());
                assert(this->operands[1]->isExpression());
                assert(this->operands[2]->isRegister());
                printInstrGeneric("bne", os, nullptr, this->operands[0]);
                break;
            case OP_BGT:
                // We assume the first operand is the symbol reference to jump to
                assert(this->operands.size() == 3);
                assert(this->operands[0]->isRegister());
                assert(this->operands[1]->isExpression());
                assert(this->operands[2]->isRegister());
                printInstrGeneric("bgt", os, nullptr, this->operands[0]);
                break;
            case OP_BLT:
                // We assume the first operand is the symbol reference to jump to
                assert(this->operands.size() == 3);
                assert(this->operands[0]->isRegister());
                assert(this->operands[1]->isExpression());
                assert(this->operands[2]->isRegister());
                printInstrGeneric("blt", os, nullptr, this->operands[0]);
                break;
            case OP_SEND_MSG:
                assert(this->operands.size() == 2);
                assert(this->operands[0]->isRegister());
                assert(this->operands[1]->isRegister());
                printInstrGeneric("send_msg", os);
                break;
            case OP_RECV_MSG:
                assert(this->operands.size() == 2);
                assert(this->operands[0]->isLocalRegister());
                assert(this->operands[1]->isRegister());
                printInstrGeneric("recv_msg", os, this->operands[0]);
                break;
            case OP_RUN_ROUTINE:
                // For running routines, we assume the firs operand is the local register to assign , the result
                // The second is the name of the routine, and all the rest of the operands are the args.
                // We make this assumption, so we avoid dealing with "variadic args"
                assert(this->operands.size() >= 2);
                assert(this->operands[0]->isRegister());
                assert(this->operands[1]->isExpression());
                printInstrGeneric("run_routine", os, this->operands[0], this->operands[1]);
                break;
            case OP_RUN_REQUEST:
                // For running routines, we assume the firs operand is the local register to assign , the result
                // The second is the name of the routine, and all the rest of the operands are the args.
                // We make this assumption, so we avoid dealing with "variadic args"
                assert(this->operands.size() >= 2);
                assert(this->operands[0]->isRegister());
                assert(this->operands[1]->isExpression());
                printInstrGeneric("run_request", os, this->operands[0], this->operands[1]);
                break;
            case OP_SUBMIT_ROUTINES:
                assert(false && "Submit routines is not supported yet!");
            case OP_JOIN_ROUTINES:
                assert(false && "Join routines is not supported yet!");
        }
    }
}