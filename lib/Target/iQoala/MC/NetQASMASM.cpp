#include "Target/iQoala/MC/iQoalaMC.h"

using namespace mlir;

namespace qoala::assembly {
    // Helper function to create a instructions with the given opcode
    static NetQASMMCInstr *create2Reg1ImmOptInstr(
        Operation *op, const NetQASMMCInstr::OpCode opCode,
            iQoalaMCOperand *reg0, iQoalaMCOperand *reg1, const std::optional<iQoalaMCOperand *> &reg2) {
            assert(reg0->isRegister() && "NetQASM 2-reg-imm instruction: operand 0 must be a register");
            assert(reg1->isRegister() && "NetQASM 2-reg-imm instruction: operand 1 must be a register");
            if (reg2.has_value()) {
                assert(reg2.value()->isImmediate() && "NetQASM 2-reg-imm instruction: operand 2 must be an immediate");
            }
            const auto instruction = new NetQASMMCInstr(op, opCode);
            instruction->addOperand(reg0);
            instruction->addOperand(reg1);
            if (reg2.has_value()) {
                instruction->addOperand(reg2.value());
            }
            return instruction;
    }

    static NetQASMMCInstr *create3RegInstr(
        Operation *op, const NetQASMMCInstr::OpCode opCode,
        iQoalaMCOperand *reg0, iQoalaMCOperand *reg1, iQoalaMCOperand *reg2) {
        assert(reg0->isRegister() && "NetQASM 3-reg instruction: operand 0 must be a register");
        assert(reg1->isRegister() && "NetQASM 3-reg instruction: operand 1 must be a register");
        assert(reg2->isRegister() && "NetQASM 3-reg instruction: operand 2 must be a register");
        const auto instruction = new NetQASMMCInstr(op, opCode);
        instruction->addOperand(reg0);
        instruction->addOperand(reg1);
        instruction->addOperand(reg2);
        return instruction;
    }

    NetQASMMCInstr *NetQASMMCInstr::createSetInstruction(Operation *op, iQoalaMCOperand *reg, iQoalaMCOperand *imm) {
        assert(reg->isRegister() && "NetQASM SET instruction: operand 0 is not quantum.");
        assert(imm->isImmediate() && "NetQASM SET instruction: operand 1 is not immediate.");
        const auto setInstruction = new NetQASMMCInstr(op, OP_SET);
        setInstruction->addOperand(reg);
        setInstruction->addOperand(imm);
        return setInstruction;
    }

    NetQASMMCInstr *NetQASMMCInstr::createAddInstruction(
        Operation *op, iQoalaMCOperand *reg0, iQoalaMCOperand *reg1, iQoalaMCOperand *reg2) {
        return create3RegInstr(op, OP_ADD, reg0, reg1, reg2);
    }

    NetQASMMCInstr *NetQASMMCInstr::createSubInstruction(
        Operation *op, iQoalaMCOperand *reg0, iQoalaMCOperand *reg1, iQoalaMCOperand *reg2) {
        return create3RegInstr(op, OP_SUB, reg0, reg1, reg2);
    }

    NetQASMMCInstr *NetQASMMCInstr::createMulInstruction(
        Operation *op, iQoalaMCOperand *reg0, iQoalaMCOperand *reg1, iQoalaMCOperand *reg2) {
        return create3RegInstr(op, OP_MUL, reg0, reg1, reg2);
    }

    NetQASMMCInstr *NetQASMMCInstr::createDivInstruction(
        Operation *op, iQoalaMCOperand *reg0, iQoalaMCOperand *reg1, iQoalaMCOperand *reg2) {
        return create3RegInstr(op, OP_DIV, reg0, reg1, reg2);
    }

    NetQASMMCInstr *NetQASMMCInstr::createRemInstruction(
        Operation *op, iQoalaMCOperand *reg0, iQoalaMCOperand *reg1, iQoalaMCOperand *reg2) {
        return create3RegInstr(op, OP_REM, reg0, reg1, reg2);
    }

    NetQASMMCInstr *NetQASMMCInstr::createJmpInstruction(Operation *op, iQoalaMCOperand *imm) {
        assert(imm->isImmediate() && "NetQASM SET instruction: operand 0 is not immediate.");
        const auto setInstruction = new NetQASMMCInstr(op, OP_JMP);
        setInstruction->addOperand(imm);
        return setInstruction;
    }
    NetQASMMCInstr *NetQASMMCInstr::createBeqInstruction(
        Operation *op, iQoalaMCOperand *reg0, iQoalaMCOperand *reg1, iQoalaMCOperand *imm) {
        return create3RegInstr(op, OP_BEQ, reg0, reg1, imm);
    }

    NetQASMMCInstr *NetQASMMCInstr::createBneInstruction(
        Operation *op, iQoalaMCOperand *reg0, iQoalaMCOperand *reg1, iQoalaMCOperand *imm) {
        return create3RegInstr(op, OP_BNE, reg0, reg1, imm);
    }

    NetQASMMCInstr *NetQASMMCInstr::createBgeInstruction(
        Operation *op, iQoalaMCOperand *reg0, iQoalaMCOperand *reg1, iQoalaMCOperand *imm) {
        return create3RegInstr(op, OP_BGE, reg0, reg1, imm);
    }

    NetQASMMCInstr *NetQASMMCInstr::createBleInstruction(
        Operation *op, iQoalaMCOperand *reg0, iQoalaMCOperand *reg1, iQoalaMCOperand *imm) {
        return create3RegInstr(op, OP_BLT, reg0, reg1, imm);
    }

    NetQASMMCInstr *NetQASMMCInstr::createLoadInstruction(Operation *op, iQoalaMCOperand *reg0, iQoalaMCOperand *reg1) {
        return create2Reg1ImmOptInstr(op, OP_LOAD, reg0, reg1, std::nullopt);
    }
    NetQASMMCInstr *NetQASMMCInstr::createStoreInstruction(Operation *op, iQoalaMCOperand *reg0, iQoalaMCOperand *reg1) {
        return create2Reg1ImmOptInstr(op, OP_STORE, reg0, reg1, std::nullopt);
    }

    void NetQASMMCInstr::print(raw_ostream &os) const {
        // The code in this function checks that the operands of the NetQASM operations
        // follow certain specification. This code is based on the NetQASM specification
        // from the Appendix F of the paper: https://doi.org/10.1088/2058-9565/ac753f
        // Additionally, the instructions emitted by this code have been cross-checked
        // with the instructions that qoala-sim can execute.
        switch (this->opCode) {
            // Allocation
            // Instructions "qalloc" and "array" are not used.
            // Initialization
            case OP_INIT:
                this->printOneRegInstr("init", os);
                break;
            case OP_SET:
                this->printOneRegOneImmInstr("set", os);
                break;
            // Memory operations
            case OP_LOAD:
            case OP_STORE:
                assert(this->operands.size() == 2);
                assert(this->operands[0]->isRegister());
                assert(this->operands[1]->isRegister());
                printStoreOrLoad(os);
                break;
            case OP_LEA:
                assert(this->operands.size() == 2);
                assert(this->operands[0]->isRegister());
                // TODO - Check if the second argument of a lea (address) is an expr or immediate
                assert(this->operands[1]->isExpression());
                this->printInstrInGenericForm("lea", os);
                break;
            // "undef" instruction is not inpreted by qoala-sim
            // Classical Logic
            case OP_JMP:
                assert(this->operands.size() == 1);
                assert(this->operands[0]->isImmediate());
                this->printInstrInGenericForm("jmp", os);
                break;
            case OP_BEZ:
                this->printOneRegOneImmInstr("bez", os);
                break;
            case OP_BNZ:
                this->printOneRegOneImmInstr("bnz", os);
                break;
            case OP_BEQ:
                this->printTwoRegsOneImmInstr("beq", os);
                break;
            case OP_BNE:
                this->printTwoRegsOneImmInstr("bne", os);
                break;
            case OP_BLT:
                this->printTwoRegsOneImmInstr("ble", os);
                break;
            case OP_BGE:
                this->printTwoRegsOneImmInstr("bge", os);
                break;
            // Classical Operations
            case OP_ADD:
                this->printThreeFourRegsInstr("add", os);
                break;
            case OP_SUB:
               this->printThreeFourRegsInstr("sub", os);
                break;
            case OP_ADDM:
                this->printThreeFourRegsInstr("addm", os, true);
                break;
            case OP_SUBM:
                this->printThreeFourRegsInstr("subm", os, true);
                break;
            // Classical Operations that were "extended" from vanilla NetQASM (v2.0)
            // (hence not in the referenced paper)
            case OP_MUL:
                this->printThreeFourRegsInstr("mul", os);
                break;
            case OP_DIV:
                this->printThreeFourRegsInstr("div", os);
                break;
            case OP_REM:
                this->printThreeFourRegsInstr("rem", os);
            break;
            // Quantum Gates
            // Single Qubit Gates
            case OP_X:
                this->printOneRegInstr("x", os);
                break;
            case OP_Y:
                this->printOneRegInstr("y", os);
                break;
            case OP_Z:
                this->printOneRegInstr("z", os);
                break;
            case OP_H:
                this->printOneRegInstr("h", os);
                break;
            case OP_S:
                this->printOneRegInstr("s", os);
                break;
            case OP_K:
                this->printOneRegInstr("k", os);
                break;
            case OP_T:
                this->printOneRegInstr("t", os);
                break;
            // Single Qubit Rotations
            case OP_ROT_X:
                this->printOneRegTwoImmInstr("rot_x", os);
                break;
            case OP_ROT_Y:
                this->printOneRegTwoImmInstr("rot_y", os);
                break;
            case OP_ROT_Z:
                this->printOneRegTwoImmInstr("rot_z", os);
                break;
            // Two Qubit Gates
            case OP_CNOT:
                this->printTwoRegInstr("cnot", os);
                break;
            case OP_CPHASE:
                this->printTwoRegInstr("cphase", os);
                break;
            // Measure
            case OP_MEAS:
                this->printTwoRegInstr("meas", os);
                break;
            // Default case
            case OP_UNKNOWN:
            default:
                this->originalOp->emitError("Op code for operation '") << *this->originalOp << "' is unknown.\n";
                break;
        }
    }

    void NetQASMMCInstr::printOneRegInstr(const std::string &mnemonic, raw_ostream &os) const {
        assert(this->operands.size() == 1);
        assert(this->operands[0]->isRegister());
        this->printInstrInGenericForm(mnemonic, os);
    }

    void NetQASMMCInstr::printTwoRegInstr(const std::string &mnemonic, raw_ostream &os) const {
        assert(this->operands.size() == 2);
        assert(this->operands[0]->isRegister());
        assert(this->operands[1]->isRegister());
        this->printInstrInGenericForm(mnemonic, os);
    }

    void NetQASMMCInstr::printOneRegTwoImmInstr(const std::string &mnemonic, raw_ostream &os) const {
        assert(this->operands.size() == 3);
        assert(this->operands[0]->isRegister());
        assert(this->operands[1]->isImmediate());
        assert(this->operands[2]->isImmediate());
        this->printInstrInGenericForm(mnemonic, os);
    }

    void NetQASMMCInstr::printOneRegOneImmInstr(const std::string &mnemonic, raw_ostream &os) const {
        assert(this->operands.size() == 2);
        assert(this->operands[0]->isRegister());
        assert(this->operands[1]->isImmediate());
        this->printInstrInGenericForm(mnemonic, os);
    }

    void NetQASMMCInstr::printTwoRegsOneImmInstr(const std::string &mnemonic, raw_ostream &os) const {
        assert(this->operands.size() == 3);
        assert(this->operands[0]->isRegister());
        assert(this->operands[1]->isRegister());
        assert(this->operands[2]->isImmediate());
        this->printInstrInGenericForm(mnemonic, os);
    }

    void NetQASMMCInstr::printThreeFourRegsInstr(const std::string &mnemonic, raw_ostream &os, const bool usesFourRegs) const {
        unsigned short numOperands;
        if (usesFourRegs) {
            numOperands = 4;
        } else {
            numOperands = 3;
        }
        assert(this->operands.size() == numOperands);
        assert(this->operands[0]->isRegister());
        assert(this->operands[1]->isRegister());
        assert(this->operands[2]->isRegister());
        if (usesFourRegs) {
            assert(this->operands[3]->isRegister());
        }
        this->printInstrInGenericForm(mnemonic, os);
    }

    void NetQASMMCInstr::printInstrInGenericForm(const std::string &mnemonic, raw_ostream &os) const {
        // Method to print a NetQASM "machine code" instruction, for the "generic" form
        os << mnemonic;
        for (const iQoalaMCOperand *operand : this->operands) {
            os << " " << *operand;
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