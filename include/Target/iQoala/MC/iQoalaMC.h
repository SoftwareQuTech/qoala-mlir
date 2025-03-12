#ifndef QOALA_MLIR_IQOALAMC_H
#define QOALA_MLIR_IQOALAMC_H

#include "Analysis/Helpers/Helpers.h"

// This file defines iQoalaMCOperand and iQoalaMCInst, which represent the
// low-level "machine code" instructions and operands.
// NetQASM, QoalaHost and QRemote instructions will be translated into these
// objects, so they can be easily printed
// These definitions are heavily inspired in the LLVM's definitions located
// in "include/llvm/MC/MCInst.h", but this is a way lighter model than the one
// presented in LLVM

// Forward declaration to avoid circular includes of iQoala.h
namespace qoala::translate {
    class ModuleTranslation;
}

namespace qoala::assembly {
    enum iQoalaRegType { LOCAL, R, C, M, Q };

    class iQoalaMC : public helpers::PrintInterface{ };

    // Forward declaration for using it inside the iQoalaMCOperand class
    class iQoalaMCInstruction;

    class iQoalaMCExpr : public iQoalaMC {
        enum ExprKind {
            INVALID,
            SYMBOL_REFERENCE,
            CONSTANT_I32,
            CONSTANT_F32
        };
    public:
        iQoalaMCExpr() : kind(INVALID), i32ConstVal(0) { }
        ~iQoalaMCExpr() override { }
        explicit iQoalaMCExpr(std::string &symbolName) : kind(SYMBOL_REFERENCE), symbolName(std::move(symbolName)) { }

        static iQoalaMCExpr *createSymbolRefExpr(std::string &symbolName);

        [[nodiscard]]
        bool isValid() const;
        [[nodiscard]]
        bool isSymbolRef() const;
        [[nodiscard]]
        bool isConstant() const;
        void print(mlir::raw_ostream &os) const override;

        static iQoalaMCExpr *createSymbolRef(const std::string &symName);
        static iQoalaMCExpr *createConstant(uint32_t value);
        static iQoalaMCExpr *createConstant(float value);
    private:
        ExprKind kind;
        union {
            std::string symbolName;
            uint32_t i32ConstVal;
            float f32ConstVal;
        };
    };

    class iQoalaRegReference {
    public :
        iQoalaRegReference() : type(R), num(0) { }
        iQoalaRegReference(const iQoalaRegType type, const uint32_t num) : type(type), num(num) { }
        ~iQoalaRegReference() = default;

        static iQoalaRegReference *createRegReference(iQoalaRegType type, uint32_t num);

        [[nodiscard]]
        std::string formatRegister() const;
        [[nodiscard]]
        iQoalaRegType getType() const { return type; }
        [[nodiscard]]
        uint32_t getNum() const { return num; }
        [[nodiscard]]
        bool isLocal() const { return type == LOCAL; }
        [[nodiscard]]
        bool isQuantum() const { return type == C || type == Q || type == M || type == R; }
    private:
        iQoalaRegType type;
        uint32_t num;
    };

    class iQoalaMCOperand : public iQoalaMC {
    public:
        enum OperandKind {
            INVALID,
            IMMEDIATE_I32,
            IMMEDIATE_F32,
            REGISTER,
            LOCAL_REGISTER,
            EXPRESSION
        };

        iQoalaMCOperand() : inst(nullptr), kind(INVALID), integerVal(0) { };
        ~iQoalaMCOperand() override = default;
        [[nodiscard]]
        bool isValid() const;
        [[nodiscard]]
        bool isImmediate() const;
        [[nodiscard]]
        bool isRegister() const;
        [[nodiscard]]
        bool isLocalRegister() const;
        [[nodiscard]]
        bool isExpression() const;

    private:
        iQoalaMCInstruction *inst;
        OperandKind kind;
        union {
            uint32_t integerVal;
            float floatingPointVal;
            iQoalaRegReference *regRef;
            iQoalaMCExpr *expression;
        };
    public:
        void print(mlir::raw_ostream &os) const override;
        void setInst(iQoalaMCInstruction *inst);

        [[nodiscard]]
        uint32_t getIntegerVal() const;
        [[nodiscard]]
        float getFloatingPointVal() const;
        [[nodiscard]]
        iQoalaRegReference *getRegRef() const;
        [[nodiscard]]
        iQoalaMCExpr *getExpression() const;

        static iQoalaMCOperand *createImmediateOperand(uint32_t val);
        static iQoalaMCOperand *createImmediateOperand(float val);
        static iQoalaMCOperand *createRegisterOperand(iQoalaRegReference *regRef);
        static iQoalaMCOperand *createExprOperand(iQoalaMCExpr *expr);
    };

    class iQoalaMCInstruction : public iQoalaMC {
    public:
        /* The first declaration of all op codes is assumed to mean "unknown" */
        explicit iQoalaMCInstruction(mlir::Operation *op) : originalOp(op), opCode(0) { }
        iQoalaMCInstruction(mlir::Operation *op, const uint32_t opCode) : originalOp(op), opCode(opCode), operands({}) { }
        iQoalaMCInstruction(const iQoalaMCInstruction &inst) : originalOp(inst.originalOp), opCode(inst.opCode), operands(inst.operands) { }
        ~iQoalaMCInstruction() override = default;

        void setOpcode(unsigned int opCode);
        [[nodiscard]]
        unsigned int getOpcode() const;

        [[nodiscard]]
        iQoalaMCOperand *getOperand(unsigned i) const;
        iQoalaMCOperand *getOperand(unsigned i);
        [[nodiscard]]
        unsigned int getNumOperands() const;

        void addOperand(iQoalaMCOperand *op);
        [[nodiscard]]
        mlir::Operation *getOriginalOp() const;
    protected:
        mlir::Operation *originalOp;
        unsigned int opCode;
        std::vector<iQoalaMCOperand *> operands;
    };

    class NetQASMMCInstr : public iQoalaMCInstruction {
    public:
        // This enum is based on the NetQASM specification
        // from the Appendix F of the paper: https://doi.org/10.1088/2058-9565/ac753f
        // Additionally, the instructions types declared in this enum have been cross-checked
        // with the instructions that qoala-sim can execute.
        enum OpCode {
            OP_UNKNOWN = 0,
            // Allocation - "qalloc" and "array" are not  supported in qoala.
            // "qalloc" semantic is implicit in the routine header, whereas arrays
            // are no longer supported in NetQASM 2.0
            // Initialization
            OP_INIT,
            OP_SET,
            // Memory Operations
            OP_LOAD,
            OP_STORE,
            // OP_UNDEF, // unused
            OP_LEA,
            // Classical logic
            OP_JMP,
            OP_BEZ,
            OP_BNZ,
            OP_BEQ,
            OP_BNE,
            OP_BLT,
            OP_BGE,
            // Classical operations
            OP_ADD,
            OP_SUB,
            OP_ADDM,
            OP_SUBM,
            OP_MUL,
            OP_DIV,
            OP_REM,
            // Quantum Gates
            // Single Qubit Gates
            OP_X,
            OP_Y,
            OP_Z,
            OP_H,
            OP_S,
            OP_K,
            OP_T,
            // Single Qubit Rotations
            OP_ROT_X,
            OP_ROT_Y,
            OP_ROT_Z,
            // Two Qubit Gates
            OP_CNOT,
            OP_CPHASE,
            // Measure
            OP_MEAS
            // Pre-measurement instructions are not implemented in netqasm library
            // and hence not handled by the qoala-sim
            // "create_epr" and "recv_epr" are not part of the NetQASM 2.0 specification
            // those instructions were replaced by "request routines"
            // Waiting instructions are tied to the "arrays" structures, hence not supported
            // in NetQASM 2.0
            // "qfree <reg>" and "return <reg>" instructions semantic are implicitly embedded
            // in the "uses:" and  "keeps:" sections of the header of the routine, hence
            // they do noe need to be emitted by the compiler.
        };

        using iQoalaMCInstruction::iQoalaMCInstruction;
        NetQASMMCInstr() : iQoalaMCInstruction(nullptr) { }

        /* Base entry point for creating QoalaHost instructions */
        static NetQASMMCInstr *build(translate::ModuleTranslation *moduleTranslation, mlir::Operation *op,
            std::optional<mlir::Value> resVal, std::optional<iQoalaRegReference *> resRegRef, OpCode opCode,
            mlir::SmallVector<iQoalaMCOperand *> &extraOperands, bool useOpOperands);

        void print(mlir::raw_ostream &os) const override;
    private:
        void printOneRegInstr(const std::string &mnemonic, mlir::raw_ostream &os) const;
        void printTwoRegInstr(const std::string &mnemonic, mlir::raw_ostream &os) const;
        void printOneRegOneImmInstr(const std::string &mnemonic, mlir::raw_ostream &os) const;
        void printOneRegTwoImmInstr(const std::string &mnemonic, mlir::raw_ostream &os) const;
        void printTwoRegsOneImmInstr(const std::string &mnemonic, mlir::raw_ostream &os) const;
        void printThreeFourRegsInstr(const std::string &mnemonic, mlir::raw_ostream &os, bool usesFourRegs = false) const;
        void printInstrInGenericForm(const std::string &mnemonic, mlir::raw_ostream &os) const;
        void printStoreOrLoad(mlir::raw_ostream &os) const;
    };

    class QoalaHostMCInstr : public iQoalaMCInstruction {
    public:
        enum OpCode {
            OP_UNKNOWN = 0,
            OP_ASSIGN_CVAL,
            OP_ADD,
            OP_SUBTRACT,
            OP_MULTIPLY_CONSTANT,
            OP_BI_COND_MULTIPLY,
            OP_JUMP,
            OP_BEQ,
            OP_BNE,
            OP_BGT,
            OP_BLT,
            OP_SEND_MSG,
            OP_RECV_MSG,
            OP_RUN_ROUTINE,
            OP_RUN_REQUEST,
            OP_SUBMIT_ROUTINES,
            OP_JOIN_ROUTINES
        };

        using iQoalaMCInstruction::iQoalaMCInstruction;
        QoalaHostMCInstr() : iQoalaMCInstruction(nullptr) { };

        /* Base entry point for creating QoalaHost instructions */
        static QoalaHostMCInstr *build(translate::ModuleTranslation *moduleTranslation, mlir::Operation *op,
            std::optional<mlir::Value> resVal, std::optional<iQoalaRegReference *> resRegRef, OpCode opCode,
            mlir::SmallVector<iQoalaMCOperand *> &extraOperands, bool useOpOperands);

        void print(mlir::raw_ostream &os) const override;
    private:
        void printInstrGeneric(const std::string &mnemonic, mlir::raw_ostream &os,
                               bool firstIsSSAReg = false,
                               bool lastIsImmediate = false) const;
    };

    class InstructionBuilder {
    public:
        template<typename Op>
        static Op *build(translate::ModuleTranslation *moduleTranslation, mlir::Operation *op,
            std::optional<mlir::Value> resVal, std::optional<iQoalaRegReference *> resRegRef, typename Op::OpCode opCode,
            mlir::SmallVector<iQoalaMCOperand *> &extraOperands, bool useOpOperands) {
            return Op::build(moduleTranslation, op, resVal, resRegRef, opCode, extraOperands, useOpOperands);
        }
    };

    mlir::raw_ostream &operator<<(mlir::raw_ostream &os, const iQoalaMCInstruction &instr);
    mlir::raw_ostream &operator<<(mlir::raw_ostream &os, const iQoalaMCOperand &oper);
    mlir::raw_ostream &operator<<(mlir::raw_ostream &os, const iQoalaMCExpr &expr);
}

#endif //QOALA_MLIR_IQOALAMC_H
