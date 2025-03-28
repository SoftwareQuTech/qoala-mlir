#include "Target/iQoala/iQoalaContext.h"

#include "llvm/Support/raw_os_ostream.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "iqoala-context"

namespace qoala::iqoala {
    iQoalaContext::iQoalaContext() {
        for (uint8_t i = 0; i < MAX_PHY_QUBITS; i++) {
            this->qubits[i] = false;
        }
    }

    uint8_t iQoalaContext::allocateQubit() {
        for (uint8_t i = 0; i < MAX_PHY_QUBITS; i++) {
            if (!this->qubits[i]) {
                this->qubits[i] = true;
                return i;
            }
        }
        return 0xFF;
    }

    void iQoalaContext::releaseQubit(const uint8_t reg) {
        this->qubits[reg] = false;
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

    uint8_t iQoalaContext::allocateClassicalSocketForRemote(const std::string &remoteID) {
        // If there is already a classical socket for this remote, simply return it:
        if (this->classicalSocketIDs.contains(remoteID)) {
            return this->classicalSocketIDs.at(remoteID);
        }
        const unsigned int newSocketID = this->classicalSocketIDs.size();
        const auto result = this->classicalSocketIDs.try_emplace(remoteID, newSocketID);
        (void) result;
        assert(result.second && "Attempting to map a remote name that is already mapped");
        return newSocketID;
    }

    uint8_t iQoalaContext::allocateEPRSSocketForRemote(const std::string &remoteID) {
        // If there is already a classical socket for this remote, simply return it:
        if (this->eprsSocketIDs.contains(remoteID)) {
            return this->eprsSocketIDs.at(remoteID);
        }
        const unsigned int newSocketID = this->eprsSocketIDs.size();
        const auto result = this->eprsSocketIDs.try_emplace(remoteID, newSocketID);
        (void) result;
        assert(result.second && "Attempting to map a remote name that is already mapped");
        return newSocketID;
    }
};