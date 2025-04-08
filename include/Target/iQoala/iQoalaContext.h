#ifndef IQOLACONTEXT_H
#define IQOLACONTEXT_H

#include "Target/iQoala/iQoala.h"
#include "Target/iQoala/MC/iQoalaMC.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/DenseMap.h"

#define MAX_PHY_QUBITS 16

namespace qoala::iqoala {
    class LocalRoutineRegisters {
    public:
        uint8_t allocateRRegistry();
        uint8_t allocateCRegistry();
        uint8_t allocateMRegistry();
        uint8_t allocateQRegistry();
    private:
        llvm::SmallVector<uint8_t, 16> rRegisters;
        llvm::SmallVector<uint8_t, 16> cRegisters;
        llvm::SmallVector<uint8_t, 16> mRegisters;
        llvm::SmallVector<uint8_t, 16> qRegisters;
    };
    /**
     * This class represents the "context" of the iQoala programs
     * It holds any static state, and mappings (such as virtual-physical
     * qubit maps).
     */
    class iQoalaContext {
    public:
        iQoalaContext();
        ~iQoalaContext() = default;

        uint8_t allocateRegister(assembly::iQoalaRegType type,
            const std::optional<LocalQuantumRoutine *> &localQuantumRoutine = std::nullopt);
        uint8_t allocateQubit();
        void releaseQubit(uint8_t reg);
        uint8_t allocateClassicalSocketForRemote(const std::string &remoteID);
        uint8_t allocateEPRSSocketForRemote(const std::string &remoteID);
        void mapValueToQubitID(const mlir::Value &value, uint8_t qubitID);
        [[nodiscard]]
        bool valueIsMappedToQubit(const mlir::Value &value) const;
        [[nodiscard]]
        uint8_t getQubitIDFor(const mlir::Value &value) const;

    private:
        // Structures to keep track of the host registers.
        llvm::SmallVector<uint8_t, 64> hostRegisters;
        llvm::DenseMap<LocalQuantumRoutine *, LocalRoutineRegisters> routinesRegisters;
        // Structure to keep track of the mlir values in the QoalaHost section that are qubit refs
        llvm::DenseMap<mlir::Value, uint8_t> valuesToQubitIDs;
        // Map for the physical qubits num->inUse
        llvm::DenseMap<uint8_t, bool> qubits;
        // Map for the remoteNames -> eprsSocketID
        llvm::StringMap<unsigned int> eprsSocketIDs;
        // Map for the remoteNames -> classicalSocketID
        llvm::StringMap<unsigned int> classicalSocketIDs;
    };
}
#endif //IQOLACONTEXT_H
