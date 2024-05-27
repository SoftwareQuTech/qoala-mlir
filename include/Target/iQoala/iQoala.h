#ifndef QOALA_MLIR_IQOALA_H
#define QOALA_MLIR_IQOALA_H

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"
#include <vector>
#include <map>

using namespace llvm;

/**
 * This file is the main implementation of the iQoala format.
 */
namespace qoala::iqoala {

    extern std::string tabStr;

    struct PrintInterface {
    public:
        PrintInterface() = default;
        virtual ~PrintInterface() = default;
        virtual void print(raw_ostream &os) const = 0;
    };

    raw_ostream &operator<<(raw_ostream &os, const PrintInterface &printable);

    struct QuantumRoutine : public PrintInterface {
    public:
        // TODO
    };

    struct LocalQuantumRoutine : public QuantumRoutine {
    public:
        void print(raw_ostream &os) const override;
        // TODO
    };

    struct RequestQuantumRoutine : public QuantumRoutine {
    public:
        void print(raw_ostream &os) const override;
        // TODO
    };

    struct Block : public PrintInterface {
        using PrintInterface::PrintInterface;
        // TODO
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

    struct NetQASMSection : public iQoalaSection {
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
    };
} // namespace qoala::iqoala

#endif //QOALA_MLIR_IQOALA_H
