#ifndef QOALA_MLIR_IQOALA_H
#define QOALA_MLIR_IQOALA_H

#include "llvm/ADT/StringRef.h"
#include <vector>
#include <map>

/**
 * This file is the main implementation of the iQoala format.
 */
namespace qoala::iqoala {

    struct PrintInterface {
    public:
        PrintInterface() = default;
        virtual ~PrintInterface() = default;
        virtual void print() = 0;
    };

    struct QuantumRoutine : public PrintInterface{
    public:
        // TODO
    };

    struct LocalQuantumRoutine : public QuantumRoutine {
    public:
        void print() override;
        // TODO
    };

    struct RequestQuantumRoutine : public QuantumRoutine {
    public:
        void print() override;
        // TODO
    };

    struct Block : public PrintInterface {
        using PrintInterface::PrintInterface;
        // TODO
    };

    struct CLBlock : public Block {
    public:
        void print() override;
        // TODO
    };

    struct CCBlock : public Block {
    public:
        void print() override;
        // TODO
    };

    struct QLBlock : public Block {
    public:
        void print() override;
        // TODO
    };

    struct QCBlock : public Block {
    public:
        void print() override;
        // TODO
    };

    struct iQoalaSection : public PrintInterface {
        // TODO
    };

    struct MetaSection : public iQoalaSection {
    public:
        void print() override;
    private:
        llvm::StringRef name;
        std::vector<int> globalParams; // TODO - Check the type of the globalParams
        std::map<std::string, unsigned int> classicalSocketsMap;
        std::map<std::string, unsigned int> eprsSocketsMap;
    };

    struct HostSection : public iQoalaSection {
    public:
        void print() override;
    private:
        std::vector<Block> hostBlocks;
    };

    struct NetQASMSection : public iQoalaSection {
    public:
        void print() override;
    private:
        // TODO - Check this
        std::vector<QuantumRoutine> routines;
    };

    struct iQoalaProgram : public PrintInterface {
    public:
        void print() override;
    private:
        MetaSection metaSection;
        HostSection routinesSection;
        NetQASMSection netQASMSection;
    };
} // namespace qoala::iqoala

#endif //QOALA_MLIR_IQOALA_H
