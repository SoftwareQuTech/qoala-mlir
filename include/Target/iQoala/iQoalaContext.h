#ifndef IQOLACONTEXT_H
#define IQOLACONTEXT_H

#include "Target/iQoala/MC/iQoalaMC.h"
#include "Target/iQoala/iQoala.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"

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
        void markOperationAsVisited(mlir::Operation *operation);
        bool isOperationVisited(const mlir::Operation *operation) const;

    private:
        // Structures to keep track of the host registers.
        llvm::SmallVector<uint8_t, 64> hostRegisters;
        llvm::SmallVector<uint8_t, 64> blockArgsRegisters;
        llvm::DenseMap<LocalQuantumRoutine *, LocalRoutineRegisters> routinesRegisters;
        // Structure to keep track of the mlir values in the QoalaHost section that are qubit refs
        llvm::DenseMap<mlir::Value, uint8_t> valuesToQubitIDs;
        // Map for the physical qubits num->inUse
        llvm::DenseMap<uint8_t, bool> qubits;
        // Map for the remoteNames -> eprsSocketID
        llvm::StringMap<uint32_t> eprsSocketIDs;
        // Map for the remoteNames -> classicalSocketID
        llvm::StringMap<uint32_t> classicalSocketIDs;
        // Set for keeping track of the visited (translated) operations of the module
        llvm::SmallPtrSet<mlir::Operation *, 32> visitedOps;
    };
} // namespace qoala::iqoala
#endif // IQOLACONTEXT_H
