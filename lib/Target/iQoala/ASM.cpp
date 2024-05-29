#include "Target/iQoala/ASM.h"

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

    void NetQASMInstruction::printInstrInGenericForm(const std::string &mnemonic, raw_ostream &os) const {
        // Method to print a NetQASM "machine code" instruction, for the "generic" form
        os << mnemonic;
        for (const iQoalaMCOperand &operand : this->operands) {
            os << " " << operand;
        }
    }

    void NetQASMInstruction::printStoreOrLoad(llvm::raw_ostream &os) const {
        // Specific case: store and loads have a slightly different format:
        switch (this->opCode) {
            case OpCode::OP_STORE:
                os << "store " << this->operands[0] << "@output[" << this->operands[1] << "]";
                break;
            case OpCode::OP_LOAD:
                os << "load " << this->operands[0] << "@input[" << this->operands[1] << "]";
                break;
            default:
                this->originalOp->emitOpError("Trying to print an instruction as a store/load which")
                << "does not have a store or load OpCode\n";
        }
    }

    void NetQASMInstruction::print(raw_ostream &os) const {
        switch (this->opCode) {
            case OpCode::OP_UNKNOWN:
                this->originalOp->emitError("Op code for operation '") << *this->originalOp << "' is unknown.\n";
                break;
            case OpCode::OP_SET:
                assert(this->operands.size() == 2);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isImmediate());
                printInstrInGenericForm("set", os);
                break;
            case OpCode::OP_ADD:
                assert(this->operands.size() == 3);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isRegister());
                assert(this->operands[2].isRegister());
                printInstrInGenericForm("add", os);
                break;
            case OpCode::OP_SUBTRACT:
                assert(this->operands.size() == 3);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isRegister());
                assert(this->operands[2].isRegister());
                printInstrInGenericForm("sub", os);
                break;
            case OpCode::OP_MULTIPLY:
                assert(this->operands.size() == 3);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isRegister());
                assert(this->operands[2].isRegister());
                printInstrInGenericForm("mul", os);
                break;
            case OpCode::OP_QUOT:
                assert(this->operands.size() == 3);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isRegister());
                assert(this->operands[2].isRegister());
                printInstrInGenericForm("quot", os);
                break;
            case OpCode::OP_REM:
                assert(this->operands.size() == 3);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isRegister());
                assert(this->operands[2].isRegister());
                printInstrInGenericForm("rem", os);
                break;
            case OpCode::OP_JUMP:
                assert(this->operands.size() == 1);
                assert(this->operands[0].isImmediate());
                printInstrInGenericForm("jmp", os);
                break;
            case OpCode::OP_BEQ:
                assert(this->operands.size() == 3);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isRegister());
                assert(this->operands[2].isImmediate());
                printInstrInGenericForm("beq", os);
                break;
            case OpCode::OP_BNE:
                assert(this->operands.size() == 3);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isRegister());
                assert(this->operands[2].isImmediate());
                printInstrInGenericForm("bne", os);
                break;
            case OpCode::OP_BGE:
                assert(this->operands.size() == 3);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isRegister());
                assert(this->operands[2].isImmediate());
                printInstrInGenericForm("bge", os);
                break;
            case OpCode::OP_BLE:
                assert(this->operands.size() == 3);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isRegister());
                assert(this->operands[2].isImmediate());
                printInstrInGenericForm("ble", os);
                break;
            case OpCode::OP_LOAD:
            case OpCode::OP_STORE:
                assert(this->operands.size() == 2);
                assert(this->operands[0].isRegister());
                assert(this->operands[1].isRegister());
                printStoreOrLoad(os);
                break;
        }
    }

    void QoalaHostInstruction::printInstruction(std::string &mnemonic, llvm::raw_ostream &os) const {
        // TODO
    }

    void QoalaHostInstruction::print(raw_ostream &os) const {
        switch (this->opCode) {
            case OP_UNKNOWN:
                this->originalOp->emitError("Op code for operation '") << *this->originalOp << "' is unknown.\n";
                break;
            case OP_ASSIGN:
                break;
            case OP_ADD:
                break;
            case OP_SUBTRACT:
                break;
            case OP_MULTIPLY_CONSTANT:
                break;
            case OP_BI_COND_MULTIPLY:
                break;
            case OP_JUMP:
                break;
            case OP_BEQ:
                break;
            case OP_BNE:
                break;
            case OP_BGT:
                break;
            case OP_BLT:
                break;
            case OP_SEND_MSG:
                break;
            case OP_RECV_MSG:
                break;
            case OP_RUN_ROUTINE:
                break;
            case OP_RUN_REQUEST:
                break;
            case OP_SUBMIT_ROUTINES:
                break;
            case OP_JOIN_ROUTINES:
                break;
        }
    }
}