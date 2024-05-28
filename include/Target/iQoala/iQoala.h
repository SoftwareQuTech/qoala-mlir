#ifndef QOALA_MLIR_IQOALA_H
#define QOALA_MLIR_IQOALA_H

#include "Target/iQoala/ASM.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"
#include <vector>
#include <map>

using namespace llvm;
using namespace qoala;
using namespace qoala::assembly;

/**
 * This file is the main implementation of the iQoala format.
 */
namespace qoala::iqoala {

    extern std::string tabStr;

    raw_ostream &operator<<(raw_ostream &os, const PrintInterface &printable);

    struct QuantumRoutine : public PrintInterface {
    public:
        // TODO
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

    enum RequestCallbackType {
        SEQUENTIAL,
        WAIT_ALL
    };

    enum VirtualIDType {
        ALL,
        INCREMENT,
        CUSTOM
    };

    struct VirtualIDs {
        VirtualIDType type;
        std::vector<unsigned int> args;
    };

    enum RequestType {
        CREATE_KEEP,
        MEASURE_DIRECTLY,
        RSP
    };

    enum RequestRole {
        CREATE,
        RECEIVE
    };

    struct RequestQuantumRoutine : public QuantumRoutine {
    public:
        void print(raw_ostream &os) const override;
        // TODO
    private:
        std::vector<std::string> returns;
        RequestCallbackType callbackType;
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

    struct Block : public PrintInterface {
        // TODO
    private:
        std::vector<QoalaHostInstruction> instructions;
    };

    struct CLBlock : public Block {
    public:
        void print(raw_ostream &os) const override;
        // TODO
    };

    struct CCBlock : public Block {
    public:
        void print(raw_ostream &os) const override;
        // TODO
    };

    struct QLBlock : public Block {
    public:
        void print(raw_ostream &os) const override;
        // TODO
    };

    struct QCBlock : public Block {
    public:
        void print(raw_ostream &os) const override;
        // TODO
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
} // namespace qoala::iqoala

#endif //QOALA_MLIR_IQOALA_H
