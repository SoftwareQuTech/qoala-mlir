#include "Target/iQoala/iQoalaContext.h"

#include "llvm/Support/raw_os_ostream.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "iqoala-context"

namespace qoala::iqoala {
    uint8_t iQoalaContext::allocateQubit() {
        // TODO - implement this
        return 0;
    }

    void iQoalaContext::releaseQubit(uint8_t reg) {
        // TODO - implement this
    }


    uint8_t iQoalaContext::allocateRegister(const assembly::iQoalaRegType type) {
        uint8_t lastAvailable = 0xFF;
        switch (type) {
            case assembly::LOCAL:
                lastAvailable = this->hostRegisters.size();
                assert(lastAvailable < 16 && "No Host register available");
                LLVM_DEBUG(llvm::dbgs() << "Allocate Host Register'" << static_cast<unsigned int>(lastAvailable) << "'\n");
                this->hostRegisters.push_back(lastAvailable);
                break;
            case assembly::C:
                lastAvailable = this->cRegisters.size();
                assert(lastAvailable < 16 && "No C register available");
                LLVM_DEBUG(llvm::dbgs() << "Allocate C Register'" << static_cast<unsigned int>(lastAvailable) << "'\n");
                this->cRegisters.push_back(lastAvailable);
                break;
            case assembly::M:
                lastAvailable = this->mRegisters.size();
                assert(lastAvailable < 16 && "No M register available");
                LLVM_DEBUG(llvm::dbgs() << "Allocate M Register'" << static_cast<unsigned int>(lastAvailable) << "'\n");
                this->mRegisters.push_back(lastAvailable);
                break;
            case assembly::Q:
                lastAvailable = this->qRegisters.size();
                assert(lastAvailable < 16 && "No Q register available");
                LLVM_DEBUG(llvm::dbgs() << "Allocate Q Register'" << static_cast<unsigned int>(lastAvailable) << "'\n");
                this->qRegisters.push_back(lastAvailable);
                break;
            case assembly::R:
                lastAvailable = this->rRegisters.size();
                assert(lastAvailable < 16 && "No R register available");
                LLVM_DEBUG(llvm::dbgs() << "Allocate R Register'" << static_cast<unsigned int>(lastAvailable) << "'\n");
                this->rRegisters.push_back(lastAvailable);
                break;
        }
        return lastAvailable;
    }
};