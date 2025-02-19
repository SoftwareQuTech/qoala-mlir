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

    class QuantumRoutine : public assembly::iQoalaMC {
    public:
        // Enum used as discriminant for subtypes.
        // This is required to use LLVM's RTTI library (dyn_cast, isa) instead oc C++'s
        // These additions are documented in https://llvm.org/docs/HowToSetUpLLVMStyleRTTI.html
        enum QuantumRoutineKind {
            QRK_Local,
            QRK_Quantum
        };
        // TODO
        QuantumRoutine() : kind(QRK_Local) { }
        explicit QuantumRoutine(const QuantumRoutineKind kind) : kind(kind) { }
        explicit QuantumRoutine(const QuantumRoutineKind kind, std::string name) :
        kind(kind), name(std::move(name)) {}
        QuantumRoutine(const QuantumRoutine &r) = default;

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

    class LocalQuantumRoutine : public QuantumRoutine {
    public:
        LocalQuantumRoutine() : QuantumRoutine(QRK_Local) {}
        explicit LocalQuantumRoutine(const mlir::StringRef newName) :
        QuantumRoutine(QRK_Local, newName.str()) {}
        LocalQuantumRoutine(const LocalQuantumRoutine &r) :
        QuantumRoutine(r.getKind(), r.getName()), usesQubits(r.usesQubits), keepsQubits(r.usesQubits),
        params(r.params), returns(r.returns), instructions(r.instructions) { }

        void print(mlir::raw_ostream &os) const override;
        // LLVM RTTI's dynamic type check
        static bool classof(const QuantumRoutine *rt) {
            return rt->getKind() == QRK_Local;
        }
        // TODO
    private:
        std::vector<unsigned int> usesQubits;
        std::vector<unsigned int> keepsQubits;
        // The names of the registries where to find the params
        std::vector<std::string> params;
        // The names of the registries that are used to return values
        std::vector<std::string> returns;
        std::vector<assembly::NetQASMMCInstr> instructions;
    };

    struct RequestCallback {
        enum RequestCallbackType { SEQUENTIAL, WAIT_ALL };
        RequestCallback() : type(SEQUENTIAL) { }
        RequestCallback(const RequestCallback &r) = default;
        explicit RequestCallback(const RequestCallbackType type) : type(type) { }
    private:
        friend mlir::raw_ostream &operator<<(mlir::raw_ostream &os, const RequestCallback &requestCallback);
        RequestCallbackType type;
    };

    struct VirtualIDs {
        enum VirtualIDType { ALL, INCREMENT, CUSTOM };
        VirtualIDs() : type(ALL) { }
        VirtualIDs(const VirtualIDs &vids) = default;
        explicit VirtualIDs(const VirtualIDType type) : type(type) { }
    private:
        friend mlir::raw_ostream &operator<<(mlir::raw_ostream &os, const VirtualIDs &virtualIDs);
        VirtualIDType type;
        std::vector<unsigned int> args = {};
    };

    struct RequestType {
        enum RequestTypeTy { CREATE_KEEP, MEASURE_DIRECTLY, RSP };
        RequestType() : type(CREATE_KEEP) { }
        RequestType(const RequestType &type) = default;
        explicit RequestType(const RequestTypeTy type) : type(type) { }
    private:
        friend mlir::raw_ostream &operator<<(mlir::raw_ostream &os, const RequestType &virtualIDs);
        RequestTypeTy type;
    };

    struct RequestRole {
        enum RequestRoleType { CREATE, RECEIVE };
        RequestRole() : type(CREATE) { }
        RequestRole(const RequestRole &role) = default;
        explicit RequestRole(const RequestRoleType type) : type(type) { }
    private:
        friend mlir::raw_ostream &operator<<(mlir::raw_ostream &os, const RequestRole &virtualIDs);
        RequestRoleType type;
    };

    class RequestQuantumRoutine : public QuantumRoutine {
    public:
        RequestQuantumRoutine() : QuantumRoutine(QRK_Quantum) { }
        RequestQuantumRoutine(const RequestQuantumRoutine &r) :
        QuantumRoutine(r.getKind(), r.getName()), returns(r.returns), requestCallback(r.requestCallback),
        callback(r.callback), remoteID(r.remoteID), eprSocketID(r.eprSocketID), numPairs(r.numPairs),
        virtualIDs(r.virtualIDs), fidelity(r.fidelity), type(r.type), requestRole(r.requestRole),
        instructions(r.instructions) { }

        void print(mlir::raw_ostream &os) const override;
        // LLVM RTTI's dynamic type check
        static bool classof(const QuantumRoutine *rt) {
            return rt->getKind() == QRK_Quantum;
        }
        // TODO
    private:
        std::vector<std::string> returns;
        RequestCallback requestCallback;
        LocalQuantumRoutine callback;
        std::string remoteID;
        unsigned int eprSocketID = 0;
        unsigned int numPairs = 0;
        VirtualIDs virtualIDs;
        float fidelity = 1.0;
        RequestType type;
        RequestRole requestRole;
        std::vector<assembly::NetQASMMCInstr> instructions;
    };

    struct BlockType {
        enum BlockTypeTy { CL, CC, QL, QC };
        BlockType() : type(CL) { }
        BlockType(const BlockType &blockType) = default;
    private:
        BlockTypeTy type;
        friend mlir::raw_ostream &operator<<(mlir::raw_ostream &os, const BlockType &blockType);
    };

    class Block : public assembly::iQoalaMC {
    public:
        Block() = default;
        Block(const Block &b) = default;

        void print(mlir::raw_ostream &os) const override;
    private:
        BlockType type;
        std::string name;
        std::vector<assembly::QoalaHostMCInstr> instructions;
    };

    /* Sections of the iQoala program */
    class MetaSection : public assembly::iQoalaMC {
    public:
        MetaSection() = default;
        MetaSection(const MetaSection &section) = default;

        void print(mlir::raw_ostream &os) const override;
        void addRemote(const std::string &remoteName);
        void setName(const std::string &programName);
    private:
        std::string name;
        // Can be i32 or f32m but for some reason, these are just "strings" in the examples from qoala-sim
        std::vector<std::string> globalParams;
        std::map<std::string, unsigned int> classicalSocketsMap;
        std::map<std::string, unsigned int> eprsSocketsMap;
    };

    class HostSection : public assembly::iQoalaMC {
    public:
        HostSection() = default;
        HostSection(const HostSection &section) = default;

        void print(mlir::raw_ostream &os) const override;
    private:
        std::vector<Block> hostBlocks;
    };

    /* These are the LOCAL quantum routines to be executed by the CPS */
    class NetQASMSection : public assembly::iQoalaMC {
    public:
        NetQASMSection() = default;
        NetQASMSection(const NetQASMSection &section) = default;

        void print(mlir::raw_ostream &os) const override;
        void addRoutine(const LocalQuantumRoutine &routine);
    private:
        std::vector<LocalQuantumRoutine> routines;
    };

    /* These are the REMOTE REQUEST quantum routines to be executed by the CPS */
    class RequestSection : public assembly::iQoalaMC {
    public:
        RequestSection() = default;
        RequestSection(const RequestSection &section) = default;

        void print(mlir::raw_ostream &os) const override;
        void addRoutine(const RequestQuantumRoutine &routine);
    private:
        std::vector<RequestQuantumRoutine> routines;
    };
    // Extra declarations for "<<" operator
    mlir::raw_ostream &operator<<(mlir::raw_ostream &os, const RequestCallback &requestCallback);
    mlir::raw_ostream &operator<<(mlir::raw_ostream &os, const VirtualIDs &virtualIDs);
    mlir::raw_ostream &operator<<(mlir::raw_ostream &os, const RequestType &virtualIDs);
    mlir::raw_ostream &operator<<(mlir::raw_ostream &os, const BlockType &block);
} // namespace qoala::iqoala

#endif //QOALA_MLIR_IQOALA_H
