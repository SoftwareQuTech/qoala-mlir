#include "Target/iQoala/MC/iQoalaMC.h"
#include "Dialect/Helpers/DialectHelpers.h"
#include "Target/iQoala/ModuleTranslation.h"

#define DEBUG_TYPE "netqasm-mc"

using namespace mlir;

namespace qoala::assembly {
    // Helper function to create instructions with the given opcode
    NetQASMMCInstr *NetQASMMCInstr::build(translate::ModuleTranslation *moduleTranslation, Operation *op,
        const std::vector<Value> &resVals, const std::vector<iQoalaRegType> &resultRegTypes,
        const OpCode opCode, SmallVector<iQoalaMCOperand *> &extraOperands, const bool useOpOperands,
        const bool appendInstruction
        ) {
        SmallVector<iQoalaMCOperand *> mcOperands;
        std::vector<iQoalaRegReference *>resRegRefs;
        const std::string localRoutineName = dialects::helpers::getParentLocalRoutineName(op);
        const auto localRoutine = moduleTranslation->getQoalaModule()->getLocalRoutineByName(localRoutineName);

        for (const iQoalaRegType resultRegType : resultRegTypes) {
            const uint8_t regNumber = moduleTranslation->getQoalaModule()->getiQoalaContext()->allocateRegister(resultRegType, localRoutine);
            resRegRefs.push_back(iQoalaRegReference::createRegReference(resultRegType, regNumber));
        }

        for (iQoalaRegReference *resRegRef : resRegRefs) {
            iQoalaMCOperand *resultRegOperand = iQoalaMCOperand::createRegisterOperand(resRegRef);

            mcOperands.push_back(resultRegOperand);
        }

        // If the operation yielded a result, it is assumed that the first operand contains the register reference for it
        for (uint32_t i = 0; i < resVals.size(); ++i) {
            moduleTranslation->mapValue(resVals[i], mcOperands[i]->getRegRef());
        }

        if (useOpOperands) {
            for (const Value operandVal : op->getOperands()) {
                iQoalaRegReference *regRef = moduleTranslation->getMappedRegReference(operandVal);
                assert(regRef && "NetQASM Instruction Builder: operand not mapped");
                assert(regRef->isQuantum() && "NetQASM Instruction Builder: mapped register is not quantum");
                mcOperands.push_back(iQoalaMCOperand::createRegisterOperand(regRef));
            }
        }

        for (iQoalaMCOperand *extraOperand : extraOperands) {
            // Extra operands are blindly added to the list after all the original operation operands
            mcOperands.push_back(extraOperand);
        }

        switch (opCode) {
            case OP_ADD:
            case OP_SUB:
            case OP_MUL:
            case OP_DIV:
            case OP_REM:
                assert(mcOperands.size() == 3 && "NetQASM instruction builder: expected 3 operands");
                assert(mcOperands[0]->isRegister() && "NetQASM 3-reg instruction: operand 0 must be a register");
                assert(mcOperands[1]->isRegister() && "NetQASM 3-reg instruction: operand 1 must be a register");
                assert(mcOperands[2]->isRegister() && "NetQASM 3-reg instruction: operand 2 must be a register");
                break;
            case OP_JMP:
                assert(mcOperands.size() == 1 && "NetQASM instruction builder: expected 1 operand");
                assert(mcOperands[0]->isExpression() && "NetQASM 1 immediate instruction: operand 0 must be an immediate");
                break;
            case OP_BEQ:
            case OP_BNE:
            case OP_BGE:
            case OP_BLT:
                assert(mcOperands.size() == 3 && "NetQASM instruction builder: expected 3 operands");
                assert(mcOperands[0]->isRegister() && "NetQASM 3-reg instruction: operand 0 must be a register");
                assert(mcOperands[1]->isRegister() && "NetQASM 3-reg instruction: operand 1 must be a register");
                assert(mcOperands[2]->isExpression() && "NetQASM 3-reg instruction: operand 2 must be an expression");
                break;
            case OP_CPHASE:
            case OP_CNOT:
            case OP_LOAD:
                assert(mcOperands.size() == 2 && "NetQASM instruction builder: expected 2 operands");
                assert(mcOperands[0]->isRegister() && "NetQASM 2-reg instruction: operand 0 must be a register");
                assert(mcOperands[1]->isRegister() && "NetQASM 2-reg instruction: operand 1 must be a register");
                break;
            case OP_MEAS:
                assert(mcOperands.size() == 2 && "NetQASM instruction builder: expected 2 operands");
                assert(mcOperands[0]->isRegister() && "NetQASM 2-reg instruction: operand 0 must be a register");
                assert(mcOperands[1]->isRegister() && "NetQASM 2-reg instruction: operand 1 must be a register");
                // Despite meas and load have the same operands, for "meas" the first operand must be the qubit
                // to measure, and not the result yielded (which is second operand).
                // To solve this issue, we simply reverse the order of "mcOperands"
                mcOperands = {mcOperands[1], mcOperands[0]};
                break;
            case OP_STORE:
            case OP_SET:
                assert(mcOperands.size() == 2 && "NetQASM instruction builder: expected 2 operands");
                assert(mcOperands[0]->isRegister() && "NetQASM 1-reg, 1-imm instruction: operand 0 is not a register.");
                assert(mcOperands[1]->isImmediate() && "NetQASM 1-reg, 1-imm instruction: operand 1 is not an immediate.");
                break;
            case OP_BEZ:
            case OP_BNZ:
                assert(mcOperands.size() == 2 && "NetQASM instruction builder: expected 3 operands");
                assert(mcOperands[0]->isRegister() && "NetQASM 3-reg instruction: operand 0 must be a register");
                assert(mcOperands[1]->isExpression() && "NetQASM 3-reg instruction: operand 2 must be an expression");
                break;
            case OP_H:
            case OP_INIT:
                assert(mcOperands.size() == 1 && "NetQASM instruction builder: expected 1 operand");
                assert(mcOperands[0]->isRegister() && "NetQASM 1-reg instruction: operand 0 must be a register");
                break;
            case OP_ROT_X:
            case OP_ROT_Y:
            case OP_ROT_Z:
                assert(mcOperands.size() == 3 && "NetQASM instruction builder: expected 3 operands");
                assert(mcOperands[0]->isRegister() && "NetQASM 1-reg, 2-imm instruction: operand 0 must be a register");
                assert(mcOperands[1]->isImmediate() && "NetQASM 1-reg, 2-imm instruction: operand 1 must be an immediate");
                assert(mcOperands[2]->isImmediate() && "NetQASM 1-reg, 2-imm instruction: operand 2 must be an immediate");
                break;
            default:
                op->emitOpError("NetQASM instruction builder: Don't know how to build operation of type: ") << opCode;
                return nullptr;
        }
        // Generic way to create a generic NetQASMInstruction with the given opCode and operands
        const auto instruction = new NetQASMMCInstr(op, opCode, /*numResults=*/1);
        for (iQoalaMCOperand *mcOperand : mcOperands) {
            instruction->addOperand(mcOperand);
        }

        if (appendInstruction) {
            localRoutine->addInstruction(instruction);
        }
        return instruction;
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
                assert(this->operands.size() == 2);
                assert(this->operands[0]->isRegister());
                assert(this->operands[1]->isRegister());
                printStoreOrLoad(os);
                break;
            case OP_STORE:
                assert(this->operands.size() == 2);
                assert(this->operands[0]->isRegister());
                assert(this->operands[1]->isImmediate());
                printStoreOrLoad(os);
                break;
            case OP_LEA:
                assert(this->operands.size() == 2);
                assert(this->operands[0]->isRegister());
                // TODO - Check if the second argument of a lea (address) is an expr or immediate
                assert(this->operands[1]->isExpression());
                this->printInstrInGenericForm("lea", os);
                break;
            // "undef" instruction is not interpreted by qoala-sim
            // Classical Logic
            case OP_JMP:
                assert(this->operands.size() == 1);
                assert(this->operands[0]->isExpression());
                this->printInstrInGenericForm("jmp", os);
                break;
            case OP_BEZ:
                assert(this->operands.size() == 2);
                assert(this->operands[0]->isRegister());
                assert(this->operands[1]->isExpression());
                assert(this->operands[1]->getExpression()->isInstructionRef());
                this->printInstrInGenericForm("bez", os);
                break;
            case OP_BNZ:
                assert(this->operands.size() == 2);
                assert(this->operands[0]->isRegister());
                assert(this->operands[1]->isExpression());
                assert(this->operands[1]->getExpression()->isInstructionRef());
                this->printInstrInGenericForm("bnz", os);
                break;
            case OP_BEQ:
                assert(this->operands.size() == 3);
                assert(this->operands[0]->isRegister());
                assert(this->operands[1]->isRegister());
                assert(this->operands[2]->isExpression());
                assert(this->operands[2]->getExpression()->isInstructionRef());
                this->printInstrInGenericForm("beq", os);
                break;
            case OP_BNE:
                assert(this->operands.size() == 3);
                assert(this->operands[0]->isRegister());
                assert(this->operands[1]->isRegister());
                assert(this->operands[2]->isExpression());
                assert(this->operands[2]->getExpression()->isInstructionRef());
                this->printInstrInGenericForm("bne", os);
                break;
            case OP_BLT:
                assert(this->operands.size() == 3);
                assert(this->operands[0]->isRegister());
                assert(this->operands[1]->isRegister());
                assert(this->operands[2]->isExpression());
                assert(this->operands[2]->getExpression()->isInstructionRef());
                this->printInstrInGenericForm("blt", os);
                break;
            case OP_BGE:
                assert(this->operands.size() == 3);
                assert(this->operands[0]->isRegister());
                assert(this->operands[1]->isRegister());
                assert(this->operands[2]->isExpression());
                assert(this->operands[2]->getExpression()->isInstructionRef());
                this->printInstrInGenericForm("bge", os);
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
                this->originalOp->emitOpError("Op code for this operation is unknown.\n");
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
                os << "store " << *this->operands[0] << " @output[" << *this->operands[1] << "]";
                break;
            case OP_LOAD:
                os << "load " << *this->operands[0] << " @input[" << *this->operands[1] << "]";
                break;
            default:
                this->originalOp->emitOpError("Trying to print an instruction as a store/load which")
                << "does not have a store or load OpCode\n";
        }
    }
}