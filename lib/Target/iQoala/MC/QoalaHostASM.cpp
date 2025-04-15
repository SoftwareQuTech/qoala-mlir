#include "Target/iQoala/MC/iQoalaMC.h"
#include "Target/iQoala/ModuleTranslation.h"

using namespace mlir;

namespace qoala::assembly {
    // Helper function to create instructions with the given opcode
    QoalaHostMCInstr *QoalaHostMCInstr::build(translate::ModuleTranslation *moduleTranslation, Operation *op,
        const std::vector<Value> &resVals, const std::vector<iQoalaRegType> &resultRegTypes,
        const OpCode opCode, SmallVector<iQoalaMCOperand *> &extraOperands, const bool useOpOperands,
        const bool appendInstruction) {
        SmallVector<iQoalaMCOperand *> mcOperands;
        SmallVector<iQoalaRegReference *>resRegRefs;

        for (const iQoalaRegType resultRegType : resultRegTypes) {
            const uint8_t regNumber = moduleTranslation->getQoalaModule()->getiQoalaContext()->allocateRegister(resultRegType);
            resRegRefs.push_back(iQoalaRegReference::createRegReference(resultRegType, regNumber));
        }

        for (iQoalaRegReference *resRegRef : resRegRefs) {
            iQoalaMCOperand *resultRegOperand = iQoalaMCOperand::createRegisterOperand(resRegRef);

            mcOperands.push_back(resultRegOperand);
        }

        // If the operation yielded a result, it is assumed that the first operand contains the register reference for it
        for (uint32_t i = 0; i < resVals.size(); ++i) {
            assert(mcOperands[i]->getRegRef()->isLocal() && "QoalaHost Instruction Builder: trying to create an instruction"
                                                            "yielding a result on a non-local register.");
            moduleTranslation->mapValue(std::nullopt, resVals[i], mcOperands[i]->getRegRef());
        }

        if (useOpOperands) {
            for (const Value operandVal : op->getOperands()) {
                iQoalaRegReference *regRef = moduleTranslation->getMappedRegReference(operandVal);
                assert(regRef && "QoalaHost Instruction Builder: operand not mapped");
                assert(regRef->isLocal() && "QoalaHost Instruction Builder: mapped register is not local");
                mcOperands.push_back(iQoalaMCOperand::createRegisterOperand(regRef));
            }
        }

        for (iQoalaMCOperand *extraOperand : extraOperands) {
            // Extra operands are blindly added to the list after all the original operation operands
            mcOperands.push_back(extraOperand);
        }

        InstrType type = UNKNOWN;
        uint32_t i = 0;
        uint32_t numResults = 1;
        iQoalaMCOperand *callee = nullptr;

        switch (opCode) {
            case OP_ASSIGN_CVAL:
                assert(mcOperands.size() == 2 && "QoalaHost instruction builder: expected 2 operands");
                assert(mcOperands[0]->isLocalRegister() && "QoalaHost instruction builder: first operand must be a local register");
                assert(mcOperands[1]->isImmediate() && "QoalaHost instruction builder: second operand must be an immediate");
                type = CL;
                break;
            case OP_ADD:
            case OP_SUBTRACT:
                assert(mcOperands.size() == 3 && "QoalaHost instruction builder: expected 3 operands");
                assert(mcOperands[0]->isLocalRegister() && "QoalaHost 3-reg instruction: operand 0 must be a local register");
                assert(mcOperands[1]->isLocalRegister() && "QoalaHost 3-reg instruction: operand 1 must be a local register");
                assert(mcOperands[2]->isLocalRegister() && "QoalaHost 3-reg instruction: operand 2 must be a local register");
                type = CL;
                break;
            case OP_MULTIPLY_CONSTANT:
                assert(mcOperands.size() == 3 && "QoalaHost instruction builder: expected 3 operands");
                assert(mcOperands[0]->isLocalRegister() && "QoalaHost 2-reg,1-imm instruction: operand 0 must be a local register");
                assert(mcOperands[1]->isLocalRegister() && "QoalaHost 2-reg,1-imm instruction: operand 1 must be a local register");
                assert(mcOperands[2]->isImmediate() && "QoalaHost 2-reg,1-imm instruction: operand 2 must be an immediate");
                type = CL;
                break;
            case OP_JUMP:
                assert(mcOperands.size() == 1 && "QoalaHost instruction builder: expected 1 operand");
                assert(mcOperands[0]->isExpression() && "QoalaHost jmp instruction: target operand must be an expression");
                assert(mcOperands[0]->getExpression()->isSymbolRef() && "QoalaHost jmp instruction: target operand must be a block reference");
                type = CL;
                break;
            case OP_BEQ:
            case OP_BNE:
            case OP_BGT:
            case OP_BLT:
                assert(mcOperands.size() == 3 && "QoalaHost instruction builder: expected 3 operands");
                assert(mcOperands[0]->isLocalRegister() && "QoalaHost 2-reg,1-block-ref instruction: operand 0 must be a register");
                assert(mcOperands[1]->isLocalRegister() && "QoalaHost 2-reg,1-block-ref instruction: operand 1 must be a register");
                assert(mcOperands[2]->isExpression() && "QoalaHost 2-reg,1-block-ref instruction: operand 2 must be an expression");
                assert(mcOperands[2]->getExpression()->isSymbolRef() && "QoalaHost 2-reg,1-block-ref instruction: operand 2 must be a block reference");
                type = CL;
                break;
            case OP_RUN_SUBROUTINE:
            case OP_RUN_REQUEST:
                // The total number of operands should be:
                // We don't consider the size of mcOperands, since for these types of operations, we expect them
                // to be included manually as "extraOperands", since arguments used as qubits must not be included
                // as function arguments.
                assert(mcOperands.size() == resRegRefs.size() + extraOperands.size() && "Qoalahost instruction builder: call operation has invalid number of operands");
                // Assert the yielded results
                for (; i < resRegRefs.size(); i++) {
                    assert(mcOperands[i]->isLocalRegister() && "Qoalahost instruction builder: call operation result is not a local register operand");
                }
                // Assert the arguments of the function
                for (; (i - resRegRefs.size()) < (extraOperands.size() - 1); i++) {
                    assert(mcOperands[i]->isLocalRegister() && "Qoalahost instruction builder: call operation argument is not a local register operand");
                }
                // Last operand must be the callee
                assert(mcOperands[i]->isExpression() && "Qoalahost instruction builder: call operation callee operand is not an expression");
                assert(mcOperands[i]->getExpression()->isSymbolRef() && "Qoalahost instruction builder: call operation callee operand is not a symbol reference");
                // We will reposition the callee operand at the beginning, so it is more convenient when printing
                callee = mcOperands[i];
                mcOperands.pop_back();
                mcOperands.insert(mcOperands.begin(), callee);
                // Set other data of the MC instruction
                numResults = resRegRefs.size();
                type = CL;
                break;
            case OP_RETURN_RESULT:
                assert(mcOperands.size() == 1 && "QoalaHost instruction builder: return_result operation returns more than 1 value");
                assert(mcOperands[0]->isLocalRegister() && "QoalaHost instruction builder: return_result operation returns a value that is not a register");
                type = CL;
                break;
            case OP_SEND_MSG:
                // TODO - assert the operands
                type = CC;
                break;
            case OP_RECV_MSG:
                // TODO - assert the operands
                type = CC;
                break;
            case OP_BI_COND_MULTIPLY:
                // TODO - assert the operands - bi_cond_mul instruction is not supported yet
                type = CL;
                break;
            default:
                op->emitOpError("QoalaHost instruction builder: Don't know how to build operation of type: ") << opCode;
                return nullptr;
        }
        assert (type != UNKNOWN && "QoalaHost instruction builder: Unknown instruction type");
        // Generic way to create a generic QoalaHostMCInstruction with the given opCode, operands and # of results
        const auto instruction = new QoalaHostMCInstr(op, opCode, numResults);
        instruction->setInstructionType(type);
        for (iQoalaMCOperand *mcOperand : mcOperands) {
            instruction->addOperand(mcOperand);
        }

        if (appendInstruction) {
            auto *block = moduleTranslation->getMappediQoalaBlock(op->getBlock());
            block->appendInstruction(instruction);
        }
        return instruction;
    }

    void QoalaHostMCInstr::setInstructionType(const InstrType instrType) {
        this->instructionType = instrType;
    }

    void QoalaHostMCInstr::printInstrGeneric(const std::string &mnemonic, raw_ostream &os,
                                             const bool firstIsSSAReg, const bool lastIsImmediate) const {
        unsigned long i = firstIsSSAReg ? 1 : 0;
        const unsigned long last = this->operands.size() - (lastIsImmediate ? 1 : 0);

        if (firstIsSSAReg) {
            os << *this->operands[0] << " = ";
        }

        os << mnemonic << "(";

        for (; i < last; i++) {
            os << *this->operands[i] << (i + 1 < last ? ", " : "");
        }
        os << ")";

        if (lastIsImmediate) {
            // Print the immediate at last is needed
            os << " : " << *this->operands[this->operands.size() - 1];
        }
    }

    void QoalaHostMCInstr::printCallInstr(const std::string &mnemonic, raw_ostream &os) const {
        uint32_t i = 1;
        uint32_t last = 0;
        if (this->numResults > 0) {
            if (this->numResults == 1) {
                os << *this->operands[1] << " = ";
                i++;
            } else {
                // The call returns multiple results, print a tuple of results.
                os << "tuple<";
                for (; i <= this->numResults; i++) {
                    os << *this->operands[i]<< (i < this->numResults ? "; " : "");
                }
                os << "> = ";
            }
        }

        os << mnemonic << "(";
        last = this->operands.size();

        // Arguments for the run_request/run_subroutine are tuples, so we use them like that
        if (i < last) {
            os << "tuple<";
            for (; i < last; i++) {
                os << *this->operands[i] << (i + 1 < last ? "; " : "");
            }
            os << ">";
        }
        os << ")";

        // Print the name of the called function
        os << " : " << *this->operands[0];
    }

    void QoalaHostMCInstr::print(raw_ostream &os) const {
        switch (this->opCode) {
            case OP_UNKNOWN:
                this->originalOp->emitOpError("Op code for this operation is unknown.\n");
                break;
            case OP_ASSIGN_CVAL:
                assert(this->operands.size() == 2);
                // We assume the first operand is the "name" of the local registry to assign to
                // And the second operand is the immediate
                assert(this->operands[0]->isLocalRegister());
                assert(this->operands[1]->isImmediate());
                printInstrGeneric("assign_cval", os, true, true);
                break;
            case OP_ADD:
                assert(this->operands.size() == 3);
                // We assume the first operand is the "name" of the local registry to assign to
                assert(this->operands[0]->isLocalRegister());
                assert(this->operands[1]->isLocalRegister());
                assert(this->operands[2]->isLocalRegister());
                printInstrGeneric("add_cval_c", os, true);
                break;
            case OP_SUBTRACT:
                assert(this->operands.size() == 3);
                // We assume the first operand is the "name" of the local registry to assign to
                assert(this->operands[0]->isLocalRegister());
                assert(this->operands[1]->isLocalRegister());
                assert(this->operands[2]->isLocalRegister());
                printInstrGeneric("sub_cval_c", os, true);
                break;
            case OP_MULTIPLY_CONSTANT:
                assert(this->operands.size() == 3);
                // We assume the first operand is the "name" of the local registry to assign to
                // And the second operand is the immediate
                assert(this->operands[0]->isLocalRegister());
                assert(this->operands[1]->isLocalRegister());
                assert(this->operands[2]->isImmediate());
                printInstrGeneric("mult_const", os, true, true);
                break;
            case OP_BI_COND_MULTIPLY:
                assert(this->operands.size() == 4);
                // We assume the first operand is the "name" of the local registry to assign to
                // And the second operand is the immediate
                assert(this->operands[0]->isLocalRegister());
                assert(this->operands[1]->isLocalRegister());
                assert(this->operands[2]->isLocalRegister());
                assert(this->operands[3]->isImmediate());
                printInstrGeneric("bcond_mult_const", os, true, true);
                break;
            case OP_JUMP:
                assert(this->operands.size() == 1);
                assert(this->operands[0]->isExpression());
                printInstrGeneric("jump", os, false, true);
                break;
            case OP_BEQ:
                // We assume the last operand is the symbol reference to jump to
                assert(this->operands.size() == 3);
                assert(this->operands[0]->isLocalRegister());
                assert(this->operands[1]->isLocalRegister());
                assert(this->operands[2]->isExpression());
                printInstrGeneric("beq", os, false, true);
                break;
            case OP_BNE:
                // We assume the last operand is the symbol reference to jump to
                assert(this->operands.size() == 3);
                assert(this->operands[0]->isLocalRegister());
                assert(this->operands[1]->isLocalRegister());
                assert(this->operands[2]->isExpression());
                printInstrGeneric("bne", os, false, true);
                break;
            case OP_BGT:
                // We assume the last operand is the symbol reference to jump to
                assert(this->operands.size() == 3);
                assert(this->operands[0]->isLocalRegister());
                assert(this->operands[1]->isLocalRegister());
                assert(this->operands[2]->isExpression());
                printInstrGeneric("bgt", os, false, true);
                break;
            case OP_BLT:
                // We assume the last operand is the symbol reference to jump to
                assert(this->operands.size() == 3);
                assert(this->operands[0]->isLocalRegister());
                assert(this->operands[1]->isLocalRegister());
                assert(this->operands[2]->isExpression());
                printInstrGeneric("blt", os, false, true);
                break;
            case OP_SEND_MSG:
                assert(this->operands.size() == 2);
                assert(this->operands[0]->isLocalRegister());
                assert(this->operands[1]->isLocalRegister());
                printInstrGeneric("send_cmsg", os);
                break;
            case OP_RECV_MSG:
                assert(this->operands.size() == 2);
                assert(this->operands[0]->isLocalRegister());
                assert(this->operands[1]->isLocalRegister());
                printInstrGeneric("recv_cmsg", os, true);
                break;
            case OP_RUN_SUBROUTINE:
                // For running routines, we assume the first operand is the name of the routine to call
                assert(this->operands[0]->isExpression());
                assert(this->operands[0]->getExpression()->isSymbolRef());
                // ALl the other operands are the yielded results, and arguments of the call
                // all of them must be local registers
                for (uint32_t i = 1; i < this->numResults; i++) {
                    assert(this->operands[i]->isLocalRegister());
                }
                printCallInstr("run_subroutine", os);
                break;
            case OP_RUN_REQUEST:
                // For running routines, we assume the first operand is the name of the routine to call
                assert(this->operands[0]->isExpression());
                assert(this->operands[0]->getExpression()->isSymbolRef());
                // ALl the other operands are the yielded results, and arguments of the call
                // all of them must be local registers
                for (uint32_t i = 1; i < this->numResults; i++) {
                    assert(this->operands[i]->isLocalRegister());
                }
                printCallInstr("run_request", os);
                break;
            case OP_RETURN_RESULT:
                printInstrGeneric("return_value", os);
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