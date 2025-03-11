#include "Target/iQoala/MC/iQoalaMC.h"
#include "Target/iQoala/ModuleTranslation.h"

using namespace mlir;

namespace qoala::assembly {
    QoalaHostMCInstr *QoalaHostMCInstr::build(Operation *op, const std::optional<Value> resVal, translate::ModuleTranslation *moduleTranslation,
        const OpCode opCode, SmallVector<iQoalaMCOperand *> &operands) {
        switch (opCode) {
            case OP_ASSIGN_CVAL:
                assert(operands.size() == 2 && "QoalaHost instruction builder: expected 2 operands");
                assert(operands[0]->isLocalRegister() && "QoalaHost instruction builder: first operand must be a local register");
                assert(operands[1]->isImmediate() && "QoalaHost instruction builder: second operand must be an immediate");
                break;
            case OP_ADD:
            case OP_SUBTRACT:
                assert(operands.size() == 3 && "QoalaHost instruction builder: expected 3 operands");
                assert(operands[0]->isLocalRegister() && "QoalaHost 3-reg instruction: operand 0 must be a local register");
                assert(operands[1]->isLocalRegister() && "QoalaHost 3-reg instruction: operand 1 must be a local register");
                assert(operands[2]->isLocalRegister() && "QoalaHost 3-reg instruction: operand 2 must be a local register");
                break;
            case OP_MULTIPLY_CONSTANT:
                assert(operands.size() == 3 && "QoalaHost instruction builder: expected 3 operands");
                assert(operands[0]->isLocalRegister() && "QoalaHost 2-reg,1-imm instruction: operand 0 must be a local register");
                assert(operands[1]->isLocalRegister() && "QoalaHost 2-reg,1-imm instruction: operand 1 must be a local register");
                assert(operands[2]->isImmediate() && "QoalaHost 2-reg,1-imm instruction: operand 2 must be an immediate");
                break;
            case OP_JUMP:
                assert(operands.size() == 1 && "QoalaHost instruction builder: expected 1 operand");
                assert(operands[0]->isExpression() && "QoalaHost jmp instruction: target operand must be an expression");
                assert(operands[0]->getExpression()->isSymbolRef() && "QoalaHost jmp instruction: target operand must be a block reference");
                break;
            case OP_BEQ:
            case OP_BNE:
            case OP_BGT:
            case OP_BLT:
                assert(operands.size() == 3 && "QoalaHost instruction builder: expected 3 operands");
                assert(operands[0]->isRegister() && "QoalaHost 2-reg,1-block-ref instruction: operand 0 must be a register");
                assert(operands[1]->isRegister() && "QoalaHost 2-reg,1-block-ref instruction: operand 1 must be a register");
                assert(operands[2]->isExpression() && "QoalaHost 2-reg,1-block-ref instruction: operand 2 must be an expression");
                assert(operands[2]->getExpression()->isSymbolRef() && "QoalaHost 2-reg,1-block-ref instruction: operand 2 must be a block reference");
            case OP_RUN_ROUTINE:
                // TODO - assert the operands.
            default:
                op->emitError("QoalaHost instruction builder: Don't know how to build operation of type: ") << opCode;
                return nullptr;
        }
        // Generic way to create a generic QoalaHostMCInstruction with the given opCode and operands
        const auto instruction = new QoalaHostMCInstr(op, opCode);
        for (iQoalaMCOperand *operand : operands) {
            instruction->addOperand(operand);
        }

        // If the operation yielded a result, it is assumed that the first operand contains the register reference for it
        if (resVal.has_value()) {
            moduleTranslation->mapValue(resVal.value(), operands[0]->getRegRef());
        }

        auto *block = moduleTranslation->getMappediQoalaBlock(op->getBlock());
        block->appendInstruction(instruction);
        return instruction;
    }

    void QoalaHostMCInstr::printInstrGeneric(const std::string &mnemonic, raw_ostream &os,
                                           const iQoalaMCOperand *ssaLocalReg,
                                           const iQoalaMCOperand *immediateExpr) const {
        auto currentElement = this->operands.begin();
        auto nextElement = this->operands.begin();
        const auto lastElement = this->operands.end();
        ++nextElement;

        if (ssaLocalReg != nullptr) {
            os << *ssaLocalReg << " = ";
            ++currentElement;
            ++nextElement;
        }

        if (immediateExpr != nullptr) {
            ++currentElement;
            ++nextElement;
        }

        os << mnemonic << " (";

        for (; currentElement != this->operands.end(); ++currentElement) {
            // We need double * since this->operands is a vector of pointers (the first star
            // dereferences the iterator, the second dereferences the operand pointer.
            os << **currentElement << (nextElement != lastElement ? ", " : "");
            ++nextElement;
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
            default:
                assert(false && "Unknown operation!");
        }
    }
}