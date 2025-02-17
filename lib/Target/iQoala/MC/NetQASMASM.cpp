#include "Target/iQoala/MC/iQoalaMC.h"

using namespace mlir;

namespace qoala::assembly {

    void NetQASMMCInstr::print(raw_ostream &os) const {
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

    void NetQASMMCInstr::printInstrInGenericForm(const std::string &mnemonic, raw_ostream &os) const {
        // Method to print a NetQASM "machine code" instruction, for the "generic" form
        os << mnemonic;
        for (const iQoalaMCOperand &operand : this->operands) {
            os << " " << operand;
        }
    }

    void NetQASMMCInstr::printStoreOrLoad(raw_ostream &os) const {
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
}