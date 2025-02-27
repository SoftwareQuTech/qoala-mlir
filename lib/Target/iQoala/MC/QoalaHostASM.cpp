#include "Target/iQoala/MC/iQoalaMC.h"

using namespace mlir;

namespace qoala::assembly {
    void QoalaHostMCInstr::printInstrGeneric(const std::string &mnemonic, llvm::raw_ostream &os,
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

        os << mnemonic << " ";

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
            case OP_ASSIGN:
                assert(this->operands.size() == 2);
                // We assume the first operand is the "name" of the local registry to assign to
                // And the second operand is the immediate
                assert(this->operands[0].isLocalRegister());
                assert(this->operands[1].isImmediate());
                printInstrGeneric("assign", os, &this->operands[0], &this->operands[1]);
                break;
            case OP_ADD:
                assert(this->operands.size() == 3);
                // We assume the first operand is the "name" of the local registry to assign to
                assert(this->operands[0].isLocalRegister());
                assert(this->operands[1].isLocalRegister());
                assert(this->operands[2].isLocalRegister());
                printInstrGeneric("add", os, &this->operands[0]);
                break;
            case OP_SUBTRACT:
                assert(this->operands.size() == 3);
                // We assume the first operand is the "name" of the local registry to assign to
                assert(this->operands[0].isLocalRegister());
                assert(this->operands[1].isLocalRegister());
                assert(this->operands[2].isLocalRegister());
                printInstrGeneric("sub", os, &this->operands[0]);
                break;
            case OP_MULTIPLY_CONSTANT:
                assert(this->operands.size() == 3);
                // We assume the first operand is the "name" of the local registry to assign to
                // And the second operand is the immediate
                assert(this->operands[0].isLocalRegister());
                assert(this->operands[1].isImmediate());
                assert(this->operands[2].isLocalRegister());
                printInstrGeneric("mult_const", os, &this->operands[0], &this->operands[1]);
                break;
            case OP_BI_COND_MULTIPLY:
                assert(this->operands.size() == 4);
                // We assume the first operand is the "name" of the local registry to assign to
                // And the second operand is the immediate
                assert(this->operands[0].isLocalRegister());
                assert(this->operands[1].isImmediate());
                assert(this->operands[2].isLocalRegister());
                assert(this->operands[3].isLocalRegister());
                printInstrGeneric("bcond_mult_const", os, &this->operands[0], &this->operands[1]);
                break;
            case OP_JUMP:
                assert(this->operands.size() == 1);
                assert(this->operands[0].isExpression());
                printInstrGeneric("jump", os, nullptr, &this->operands[0]);
                break;
            case OP_BEQ:
                // We assume the first operand is the symbol reference to jump to
                assert(this->operands.size() == 3);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isExpression());
                assert(this->operands[2].isRegister());
                printInstrGeneric("beq", os, nullptr, &this->operands[0]);
                break;
            case OP_BNE:
                // We assume the first operand is the symbol reference to jump to
                assert(this->operands.size() == 3);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isExpression());
                assert(this->operands[2].isRegister());
                printInstrGeneric("bne", os, nullptr, &this->operands[0]);
                break;
            case OP_BGT:
                // We assume the first operand is the symbol reference to jump to
                assert(this->operands.size() == 3);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isExpression());
                assert(this->operands[2].isRegister());
                printInstrGeneric("bgt", os, nullptr, &this->operands[0]);
                break;
            case OP_BLT:
                // We assume the first operand is the symbol reference to jump to
                assert(this->operands.size() == 3);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isExpression());
                assert(this->operands[2].isRegister());
                printInstrGeneric("blt", os, nullptr, &this->operands[0]);
                break;
            case OP_SEND_MSG:
                assert(this->operands.size() == 2);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isRegister());
                printInstrGeneric("send_msg", os);
                break;
            case OP_RECV_MSG:
                assert(this->operands.size() == 2);
                assert(this->operands[0].isLocalRegister());
                assert(this->operands[1].isRegister());
                printInstrGeneric("recv_msg", os, &this->operands[0]);
                break;
            case OP_RUN_ROUTINE:
                // For running routines, we assume the firs operand is the local register to assign , the result
                // The second is the name of the routine, and all the rest of the operands are the args.
                // We make this assumption, so we avoid dealing with "variadic args"
                assert(this->operands.size() >= 2);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isExpression());
                printInstrGeneric("run_routine", os, &this->operands[0], &this->operands[1]);
                break;
            case OP_RUN_REQUEST:
                // For running routines, we assume the firs operand is the local register to assign , the result
                // The second is the name of the routine, and all the rest of the operands are the args.
                // We make this assumption, so we avoid dealing with "variadic args"
                assert(this->operands.size() >= 2);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isExpression());
                printInstrGeneric("run_request", os, &this->operands[0], &this->operands[1]);
                break;
            case OP_SUBMIT_ROUTINES:
                assert(false && "Submit routines is not supported yet!");
            case OP_JOIN_ROUTINES:
                assert(false && "Join routines is not supported yet!");
        }
    }
}