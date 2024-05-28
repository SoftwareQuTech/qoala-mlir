#ifndef QOALA_MLIR_IQOALA_H
#define QOALA_MLIR_IQOALA_H

#include "Target/iQoala/ASM.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"
#include <vector>
#include <map>

using namespace qoala::assembly;

/**
 * This file is the main implementation of the iQoala format.
 */
namespace qoala::iqoala {

    extern std::string tabStr;

    struct QuantumRoutine : public PrintInterface {
    public:
        // TODO
        [[nodiscard]]
        std::string getName() const { return name; }
    protected:
        std::string name;
    };

    struct LocalQuantumRoutine : public QuantumRoutine {
    public:
        void print(raw_ostream &os) const override;
        // TODO
    private:
        std::vector<unsigned int> usesQubits;
        std::vector<unsigned int> keepsQubits;
        // The names of the registries where to find the params
        std::vector<std::string> params;
        // The names of the registries that are used to return values
        std::vector<std::string> returns;
        std::vector<NetQASMInstruction> instructions;
    };

    struct RequestCallback {
    public:
        enum RequestCallbackType { SEQUENTIAL, WAIT_ALL };
        RequestCallback() : type(SEQUENTIAL) { };
        RequestCallback(RequestCallbackType type) : type(type) { };
    private:
        friend raw_ostream &operator<<(raw_ostream &os, const RequestCallback &requestCallback);
        RequestCallbackType type;
    };

    struct VirtualIDs {
    public:
        enum VirtualIDType { ALL, INCREMENT, CUSTOM };
        VirtualIDs() : type(ALL) { };
        VirtualIDs(VirtualIDType type) : type(type) { };
    private:
        friend raw_ostream &operator<<(raw_ostream &os, const VirtualIDs &virtualIDs);
        VirtualIDType type;
        std::vector<unsigned int> args;
    };

    struct RequestType {
    public:
        enum RequestTypeTy { CREATE_KEEP, MEASURE_DIRECTLY, RSP };
        RequestType() : type(CREATE_KEEP) { };
        RequestType(RequestTypeTy type) : type(type) { };
    private:
        friend raw_ostream &operator<<(raw_ostream &os, const RequestType &virtualIDs);
        RequestTypeTy type;
    };

    struct RequestRole {
    public:
        enum RequestRoleType { CREATE, RECEIVE };
        RequestRole() : type(CREATE) { };
        RequestRole(RequestRoleType type) : type(type) { };
    private:
        friend raw_ostream &operator<<(raw_ostream &os, const RequestRole &virtualIDs);
        RequestRoleType type;
    };

    struct RequestQuantumRoutine : public QuantumRoutine {
    public:
        void print(raw_ostream &os) const override;
        // TODO
    private:
        std::vector<std::string> returns;
        RequestCallback requestCallback;
        LocalQuantumRoutine callback;
        std::string remoteID;
        unsigned int eprSocketID;
        unsigned int numPairs;
        VirtualIDs virtualIDs;
        float fidelity;
        RequestType type;
        RequestRole requestRole;
        std::vector<NetQASMInstruction> instructions;
    };

    struct BlockType {
    public:
        enum BlockTypeTy { CL, CC, QL, QC };
    private:
        BlockTypeTy type;
        friend raw_ostream &operator<<(raw_ostream &os, const BlockType &blockType);
    };

    struct Block : public PrintInterface {
    public:
        void print(raw_ostream &os) const override;
        // TODO
    private:
        BlockType type;
        std::vector<QoalaHostInstruction> instructions;
    };

    struct iQoalaSection : public PrintInterface {
        // TODO
    };

    struct MetaSection : public iQoalaSection {
    public:
        void print(raw_ostream &os) const override;
    private:
        llvm::StringRef name;
        // Can be i32 or f32m but for some reason, these are just "strings" in the examples from qoala-sim
        std::vector<std::string> globalParams;
        std::map<std::string, unsigned int> classicalSocketsMap;
        std::map<std::string, unsigned int> eprsSocketsMap;
    };

    struct HostSection : public iQoalaSection {
    public:
        void print(raw_ostream &os) const override;
    private:
        std::vector<Block> hostBlocks;
    };

    /* These are the LOCAL quantum routines to be executed by the CPS */
    struct NetQASMSection : public iQoalaSection {
    public:
        void print(raw_ostream &os) const override;
    private:
        // TODO - Check this
        std::vector<QuantumRoutine> routines;
    };

    /* These are the REMOTE REQUEST quantum routines to be executed by the CPS */
    struct RequestSection : public iQoalaSection {
    public:
        void print(raw_ostream &os) const override;
    private:
        // TODO - Check this
        std::vector<QuantumRoutine> routines;
    };

    /**
     * Class that represents the program in iQoala format
     * The main entry point of the "print" function is here.
     */
    struct iQoalaProgram : public PrintInterface {
    public:
        void print(raw_ostream &os) const override;
    private:
        MetaSection metaSection;
        HostSection hostSection;
        NetQASMSection netQASMSection;
        RequestSection requestSection;
    };

    // Extra declarations for "<<" operator
    raw_ostream &operator<<(raw_ostream &os, const PrintInterface &printable);
    raw_ostream &operator<<(raw_ostream &os, const RequestCallback &requestCallback);
    raw_ostream &operator<<(raw_ostream &os, const VirtualIDs &virtualIDs);
    raw_ostream &operator<<(raw_ostream &os, const RequestType &virtualIDs);
    raw_ostream &operator<<(raw_ostream &os, const BlockType &block);
} // namespace qoala::iqoala

#endif //QOALA_MLIR_IQOALA_H
