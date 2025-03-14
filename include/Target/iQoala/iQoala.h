#ifndef QOALA_MLIR_IQOALA_H
#define QOALA_MLIR_IQOALA_H

#include "Target/iQoala/MC/iQoalaMC.h"
#include "llvm/ADT/StringRef.h"
#include <utility>
#include <vector>
#include <map>

/**
 * This file is the main implementation of the iQoala format.
 * The classes here also inherit from the "MC" base object, since they
 * are "ready" to be formatted as iQoala ASM. This also implies that all
 * these concrete classes *need* to implement the "print" method
 */
namespace qoala::iqoala {

    extern std::string tabStr;

    /* Abstract class for a generic quantum routine. A concrete instance can be either
     * a LocalQuantum Routine or a RequestQuantumRoutine */
    class QuantumRoutine : public assembly::iQoalaMC {
    public:
        // Enum used as discriminant for subtypes.
        // This is required to use LLVM's RTTI library (dyn_cast, isa) instead oc C++'s
        // These additions are documented in https://llvm.org/docs/HowToSetUpLLVMStyleRTTI.html
        enum QuantumRoutineKind {
            QRK_LOCAL,
            QRK_QUANTUM
        };
        QuantumRoutine() : kind(QRK_LOCAL) { }
        explicit QuantumRoutine(const QuantumRoutineKind kind) : kind(kind) { }
        explicit QuantumRoutine(const QuantumRoutineKind kind, std::string name) :
        kind(kind), name(std::move(name)) {}
        QuantumRoutine(const QuantumRoutine &r) = default;
        ~QuantumRoutine() override = default;

        [[nodiscard]]
        std::string getName() const { return name; }

        // LLVM RTTI
        [[nodiscard]]
        QuantumRoutineKind getKind() const { return kind; }
    private:
        // LLVM RTTI kind
        const QuantumRoutineKind kind;
    protected:
        std::string name;
    };

    /* A class representing a local quantum routine. These routines only host a list of
     * instructions, and they do not have the concept of "blocks" */
    class LocalQuantumRoutine : public QuantumRoutine {
    public:
        LocalQuantumRoutine() : QuantumRoutine(QRK_LOCAL) {}
        explicit LocalQuantumRoutine(const mlir::StringRef newName) :
            QuantumRoutine(QRK_LOCAL, newName.str()) {}
            LocalQuantumRoutine(const LocalQuantumRoutine &r) :
            QuantumRoutine(r.getKind(), r.getName()), usesQubits(r.usesQubits), keepsQubits(r.usesQubits),
            params(r.params), returns(r.returns), instructions(r.instructions) { }
        ~LocalQuantumRoutine() override {
            for (const auto instruction : this->instructions) {
                delete instruction;
            }
        }

        static LocalQuantumRoutine *createLocalRoutine(llvm::StringRef name);

        void addInstruction(assembly::NetQASMMCInstr *instruction);
        void addArgument(const std::string &argName);
        void addReturnValue(const std::string &valName);

        void print(mlir::raw_ostream &os) const override;
        // LLVM RTTI's dynamic type check
        static bool classof(const QuantumRoutine *rt) {
            return rt->getKind() == QRK_LOCAL;
        }
    private:
        // The list of physical qubits this routine uses
        std::vector<unsigned int> usesQubits;
        // The list of physical qubits this routine keeps (and does not free automatically)
        std::vector<unsigned int> keepsQubits;
        // The names of the registries where to find the params
        std::vector<std::string> params;
        // The names of the registries that are used to return values
        std::vector<std::string> returns;
        // The list of NetQASM MC instructions for this local quantum routine
        std::vector<assembly::NetQASMMCInstr *> instructions;
    };

    struct VirtualIDs {
        enum VirtualIDType { ALL, INCREMENT, CUSTOM };
        VirtualIDs() : type(ALL) { }
        VirtualIDs(const VirtualIDs &vids) = default;
        explicit VirtualIDs(const VirtualIDType type) : type(type) { }
        ~VirtualIDs() = default;
    private:
        friend mlir::raw_ostream &operator<<(mlir::raw_ostream &os, const VirtualIDs &virtualIDs);
        VirtualIDType type;
        std::vector<unsigned int> args = {};
    };

    /* A class representing a single request quantum routine */
    class RequestQuantumRoutine : public QuantumRoutine {
    public:
        enum RequestCallback { SEQUENTIAL, WAIT_ALL };
        enum RequestType { CREATE_KEEP, MEASURE_DIRECTLY, RSP };
        enum RequestRole { CREATE, RECEIVE };

        explicit RequestQuantumRoutine(const llvm::StringRef name) :
            QuantumRoutine(QRK_QUANTUM, name.str()), requestCallback(SEQUENTIAL),
            callback(nullptr), type(CREATE_KEEP), requestRole(CREATE) { }
        RequestQuantumRoutine(const RequestQuantumRoutine &r) :
            QuantumRoutine(r.getKind(), r.getName()), returns(r.returns), requestCallback(r.requestCallback),
            callback(r.callback), remoteID(r.remoteID), eprSocketID(r.eprSocketID), numPairs(r.numPairs),
            virtualIDs(r.virtualIDs), fidelity(r.fidelity), type(r.type), requestRole(r.requestRole),
            instructions(r.instructions) { }
        ~RequestQuantumRoutine() override {
           for (const auto instruction : this->instructions) {
               delete instruction;
           }
        }

        static RequestQuantumRoutine *createRequestRoutine(llvm::StringRef name);

        void print(mlir::raw_ostream &os) const override;
        // LLVM RTTI's dynamic type check
        static bool classof(const QuantumRoutine *rt) {
            return rt->getKind() == QRK_QUANTUM;
        }
    private:
        // The list of returns
        std::vector<std::string> returns;
        // The request callback type
        RequestCallback requestCallback;
        // The local quantum routine to invoke as callback
        LocalQuantumRoutine *callback;
        // The name of the remote
        std::string remoteID;
        // The id of the EPR socket to use
        unsigned int eprSocketID = 0;
        // The number of the pairs provided by this request routine
        unsigned int numPairs = 0;
        // The method to handle virtual IDs for the entangled qubits
        VirtualIDs virtualIDs;
        // The minimum fidelity value for the communication
        float fidelity = 1.0;
        // The request type
        RequestType type;
        // The Request role for this client
        RequestRole requestRole;
        // The set of NetQASM instructions for this request routine
        // This list SHOULD be unused! (i.e. always empty)
        std::vector<assembly::NetQASMMCInstr *> instructions;
    };


    /* A class representing a block in the "qoalahost" section */
    class Block : public assembly::iQoalaMC {
    public:
        enum BlockType { CL, CC, QL, QC };
        Block() : type(CL) { }
        Block(const Block &b) = default;
        ~Block() override {
            for (const auto instruction : this->instructions) {
                delete instruction;
            }
        }

        void print(mlir::raw_ostream &os) const override;
        void appendInstruction(assembly::QoalaHostMCInstr *instruction);
    private:
        // type of the Block (CL, CC, QL, QC)
        BlockType type;
        // Name of the block
        std::string name;
        // List of QoalaHostMCInstr that compose the block
        std::vector<assembly::QoalaHostMCInstr *> instructions;
        // List of references for predecessor and successor blocks
        // In the meantime, these vectors are unused, but will be populated
        // when translating the CFG information.
        std::vector<Block *> predecessors;
        std::vector<Block *> successors;
    };

    /* Sections of the iQoala program */
    class MetaSection : public assembly::iQoalaMC {
    public:
        MetaSection() = default;
        MetaSection(const MetaSection &section) = default;
        ~MetaSection() override = default;

        void print(mlir::raw_ostream &os) const override;
        void addRemote(const std::string &remoteName);
        void setName(const std::string &programName);
    private:
        // Name of the iQoala program
        std::string name;
        // Global params of the whole iQoala program
        // Can be i32 or f32, but for some reason, these are just "strings" in the examples from qoala-sim
        std::vector<std::string> globalParams;
        // Maps for classical ane epr sockets Remote_name (str) -> socket_id (int)
        std::map<std::string, unsigned int> classicalSocketsMap;
        std::map<std::string, unsigned int> eprsSocketsMap;
    };

    class HostSection : public assembly::iQoalaMC {
    public:
        HostSection() = default;
        HostSection(const HostSection &section) = default;
        ~HostSection() override {
            for (const auto block : this->hostBlocks) {
                delete block;
            }
        }

        Block *createNewBlock();

        void print(mlir::raw_ostream &os) const override;
    private:
        // The host section only contains a list of "Blocks".
        // Each block contains the QoalaHost instructions to execute.
        std::vector<Block *> hostBlocks;
    };

    /* These are the LOCAL quantum routines to be executed by the CPS */
    class NetQASMSection : public assembly::iQoalaMC {
    public:
        NetQASMSection() = default;
        NetQASMSection(const NetQASMSection &section) = default;
        ~NetQASMSection() override {
            for (const auto routine : this->routines) {
                delete routine;
            }
        }

        void print(mlir::raw_ostream &os) const override;
        void addRoutine(LocalQuantumRoutine *routine);

        [[nodiscard]]
        std::vector<LocalQuantumRoutine *> getRoutines() const;
    private:
        // The NetQASM section simply contains a list of LocalQuantumRoutines
        std::vector<LocalQuantumRoutine *> routines;
    };

    /* These are the REMOTE REQUEST quantum routines to be executed by the CPS */
    class RequestSection : public assembly::iQoalaMC {
    public:
        RequestSection() = default;
        RequestSection(const RequestSection &section) = default;
        ~RequestSection() override {
            for (const auto routine : this->routines) {
                delete routine;
            }
        }

        void print(mlir::raw_ostream &os) const override;
        void addRoutine(RequestQuantumRoutine *routine);
    private:
        // The request section simply contains a list of RequestQuantumRoutines
        std::vector<RequestQuantumRoutine *> routines;
    };
    // Extra declarations for "<<" operator
    mlir::raw_ostream &operator<<(mlir::raw_ostream &os, RequestQuantumRoutine::RequestCallback requestCallback);
    mlir::raw_ostream &operator<<(mlir::raw_ostream &os, const VirtualIDs &virtualIDs);
    mlir::raw_ostream &operator<<(mlir::raw_ostream &os, RequestQuantumRoutine::RequestType virtualIDs);
    mlir::raw_ostream &operator<<(mlir::raw_ostream &os, Block::BlockType block);
} // namespace qoala::iqoala

#endif //QOALA_MLIR_IQOALA_H
