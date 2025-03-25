#ifndef IQOLACONTEXT_H
#define IQOLACONTEXT_H

#include "Target/iQoala/MC/iQoalaMC.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/DenseMap.h"

#define MAX_PHY_QUBITS 16

namespace qoala::iqoala {
    /**
     * This class represents the "context" of the iQoala programs
     * It holds any static state, and mappings (such as virtual-physical
     * qubit maps).
     */
    class iQoalaContext {
    public:
        iQoalaContext();
        ~iQoalaContext() = default;

        uint8_t allocateRegister(assembly::iQoalaRegType type);
        uint8_t allocateQubit();
        void releaseQubit(uint8_t reg);

    private:
        // Structures to keep track of the used registers.
        llvm::SmallVector<uint8_t, 64> hostRegisters;
        llvm::SmallVector<uint8_t, 16> rRegisters;
        llvm::SmallVector<uint8_t, 16> cRegisters;
        llvm::SmallVector<uint8_t, 16> mRegisters;
        llvm::SmallVector<uint8_t, 16> qRegisters;
        // Map for the physical qubits num->inUse
        llvm::DenseMap<uint8_t, bool> qubits;
    };
}
#endif //IQOLACONTEXT_H
