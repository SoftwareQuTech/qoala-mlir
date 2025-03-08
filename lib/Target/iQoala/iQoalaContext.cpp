#include "Target/iQoala/iQoalaContext.h"

#include "llvm/Support/raw_os_ostream.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "iqoala-context"

namespace qoala::iqoala {
    uint8_t iQoalaContext::allocateHostRegister() {
        const uint8_t lastAvailable = this->hostRegisters.size();
        assert(lastAvailable < 16 && "No Host register available");
        LLVM_DEBUG(llvm::dbgs() << "allocateHostRegister'" << static_cast<unsigned int>(lastAvailable) << "'\n");
        this->hostRegisters.push_back(lastAvailable);
        return lastAvailable;
    }
    uint8_t iQoalaContext::allocateRRegister() {
        const uint8_t lastAvailable = this->rRegisters.size();
        assert(lastAvailable < 16 && "No R register available");
        this->rRegisters.push_back(lastAvailable);
        return lastAvailable;
    }

    uint8_t iQoalaContext::allocateCRegister() {
        const uint8_t lastAvailable = this->cRegisters.size();
        assert(lastAvailable < 16 && "No C register available");
        this->cRegisters.push_back(lastAvailable);
        return lastAvailable;
    }

    uint8_t iQoalaContext::allocateMRegister() {
        const uint8_t lastAvailable = this->mRegisters.size();
        assert(lastAvailable < 16 && "No M register available");
        this->mRegisters.push_back(lastAvailable);
        return lastAvailable;
    }

    uint8_t iQoalaContext::allocateQRegister() {
        const uint8_t lastAvailable = this->qRegisters.size();
        assert(lastAvailable < 16 && "No Q register available");
        this->qRegisters.push_back(lastAvailable);
        return lastAvailable;
    }
};