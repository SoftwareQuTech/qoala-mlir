#include "Target/iQoala/MC/iQoalaMC.h"

namespace qoala::assembly {
    void iQoalaExpr::print(raw_ostream &os) const {
        // TODO
    }

    static std::string formatRegister(const iQoalaMCOperand::iQoalaRegReference &registerRef) {
        std::stringstream formattedRegStr;
        switch (registerRef.type) {
            case R:
                formattedRegStr << "R" << registerRef.num;
                break;
            case C:
                formattedRegStr << "C" << registerRef.num;
                break;
            case M:
                formattedRegStr << "M" << registerRef.num;
                break;
            case Q:
                formattedRegStr << "Q" << registerRef.num;
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
                os << this->floatingPointVal;
                break;
            case REGISTER:
                os << formatRegister(this->regRef);
                break;
            case EXPRESSION:
                assert(false && "Using an expression as operand is not supported yet!\n");
        }
    }
}