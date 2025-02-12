#include "Target/iQoala/ASM.h"
#include <iterator>

using namespace mlir;

namespace qoala::assembly {
    // Implementations of the "<<" operator
    raw_ostream &operator<<(raw_ostream &os, const iQoalaMCInstruction &instr) {
        instr.print(os);
        return os;
    }
    raw_ostream &operator<<(raw_ostream &os, const iQoalaMCOperand &oper) {
        oper.print(os);
        return os;

    }
    raw_ostream &operator<<(raw_ostream &os, const iQoalaExpr &expr) {
        expr.print(os);
        return os;

    }

    void NetQASMBaseInstr::printInstrInGenericForm(const std::string &mnemonic, raw_ostream &os) const {
        // Method to print a NetQASM "machine code" instruction, for the "generic" form
        os << mnemonic;
        for (const iQoalaMCOperand &operand : this->operands) {
            os << " " << operand;
        }
    }

    void NetQASMBaseInstr::printStoreOrLoad(raw_ostream &os) const {
        // Specific case: store and loads have a slightly different format:
        switch (this->opCode) {
            case OP_STORE:
                os << "store " << this->operands[0] << "@output[" << this->operands[1] << "]";
                break;
            case OP_LOAD:
                os << "load " << this->operands[0] << "@input[" << this->operands[1] << "]";
                break;
            default:
                this->originalOp->emitOpError("Trying to print an instruction as a store/load which")
                << "does not have a store or load OpCode\n";
        }
    }

    void NetQASMBaseInstr::print(raw_ostream &os) const {
        switch (this->opCode) {
            case OP_UNKNOWN:
                this->originalOp->emitError("Op code for operation '") << *this->originalOp << "' is unknown.\n";
                break;
            case OP_SET:
                assert(this->operands.size() == 2);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isImmediate());
                printInstrInGenericForm("set", os);
                break;
            case OP_ADD:
                assert(this->operands.size() == 3);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isRegister());
                assert(this->operands[2].isRegister());
                printInstrInGenericForm("add", os);
                break;
            case OP_SUBTRACT:
                assert(this->operands.size() == 3);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isRegister());
                assert(this->operands[2].isRegister());
                printInstrInGenericForm("sub", os);
                break;
            case OP_MULTIPLY:
                assert(this->operands.size() == 3);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isRegister());
                assert(this->operands[2].isRegister());
                printInstrInGenericForm("mul", os);
                break;
            case OP_QUOT:
                assert(this->operands.size() == 3);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isRegister());
                assert(this->operands[2].isRegister());
                printInstrInGenericForm("quot", os);
                break;
            case OP_REM:
                assert(this->operands.size() == 3);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isRegister());
                assert(this->operands[2].isRegister());
                printInstrInGenericForm("rem", os);
                break;
            case OP_JUMP:
                assert(this->operands.size() == 1);
                assert(this->operands[0].isImmediate());
                printInstrInGenericForm("jmp", os);
                break;
            case OP_BEQ:
                assert(this->operands.size() == 3);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isRegister());
                assert(this->operands[2].isImmediate());
                printInstrInGenericForm("beq", os);
                break;
            case OP_BNE:
                assert(this->operands.size() == 3);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isRegister());
                assert(this->operands[2].isImmediate());
                printInstrInGenericForm("bne", os);
                break;
            case OP_BGE:
                assert(this->operands.size() == 3);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isRegister());
                assert(this->operands[2].isImmediate());
                printInstrInGenericForm("bge", os);
                break;
            case OP_BLE:
                assert(this->operands.size() == 3);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isRegister());
                assert(this->operands[2].isImmediate());
                printInstrInGenericForm("ble", os);
                break;
            case OP_LOAD:
            case OP_STORE:
                assert(this->operands.size() == 2);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isRegister());
                printStoreOrLoad(os);
                break;
        }
    }

    void QoalaHostInstr::printInstrGeneric(const std::string &mnemonic, llvm::raw_ostream &os,
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

    void QoalaHostInstr::print(raw_ostream &os) const {
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