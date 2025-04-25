#include "Target/iQoala/iQoalaContext.h"

#include "llvm/Support/raw_os_ostream.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "iqoala-context"

using namespace mlir;

namespace qoala::iqoala {
    uint8_t LocalRoutineRegisters::allocateCRegistry() {
        const uint8_t lastAvailable = this->cRegisters.size();
        assert(lastAvailable < 16 && "No C register available");
        LLVM_DEBUG(llvm::dbgs() << "Allocate C Register '" << static_cast<uint32_t>(lastAvailable) << "'\n");
        this->cRegisters.push_back(lastAvailable);
        return lastAvailable;
    }

    uint8_t LocalRoutineRegisters::allocateRRegistry() {
        const uint8_t lastAvailable = this->rRegisters.size();
        assert(lastAvailable < 16 && "No R register available");
        LLVM_DEBUG(llvm::dbgs() << "Allocate R Register '" << static_cast<uint32_t>(lastAvailable) << "'\n");
        this->rRegisters.push_back(lastAvailable);
        return lastAvailable;
    }

    uint8_t LocalRoutineRegisters::allocateQRegistry() {
        const uint8_t lastAvailable = this->qRegisters.size();
        assert(lastAvailable < 16 && "No Q register available");
        LLVM_DEBUG(llvm::dbgs() << "Allocate Q Register '" << static_cast<uint32_t>(lastAvailable) << "'\n");
        this->qRegisters.push_back(lastAvailable);
        return lastAvailable;
    }

    uint8_t LocalRoutineRegisters::allocateMRegistry() {
        const uint8_t lastAvailable = this->mRegisters.size();
        assert(lastAvailable < 16 && "No M register available");
        LLVM_DEBUG(llvm::dbgs() << "Allocate M Register '" << static_cast<uint32_t>(lastAvailable) << "'\n");
        this->mRegisters.push_back(lastAvailable);
        return lastAvailable;
    }


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

    uint8_t iQoalaContext::allocateRegister(const assembly::iQoalaRegType type,
        const std::optional<LocalQuantumRoutine *> &localQuantumRoutine) {
        uint8_t lastAvailable = 0xFF;
        switch (type) {
            case assembly::LOCAL:
                lastAvailable = this->hostRegisters.size();
                assert(!localQuantumRoutine.has_value() && "Trying to allocate a local registry for a Local quanutm routine.");
                assert(lastAvailable < 64 && "No Host register available");
                LLVM_DEBUG(llvm::dbgs() << "Allocate Host Register '" << static_cast<uint32_t>(lastAvailable) << "'\n");
                this->hostRegisters.push_back(lastAvailable);
                break;
            case assembly::C:
                if (!this->routinesRegisters.contains(localQuantumRoutine.value())) {
                    this->routinesRegisters.try_emplace(localQuantumRoutine.value(), LocalRoutineRegisters{});
                }
                lastAvailable = this->routinesRegisters[localQuantumRoutine.value()].allocateCRegistry();
                break;
            case assembly::M:
                if (!this->routinesRegisters.contains(localQuantumRoutine.value())) {
                    this->routinesRegisters.try_emplace(localQuantumRoutine.value(), LocalRoutineRegisters{});
                }
                lastAvailable =this->routinesRegisters[localQuantumRoutine.value()].allocateMRegistry();
                break;
            case assembly::Q:
                if (!this->routinesRegisters.contains(localQuantumRoutine.value())) {
                    this->routinesRegisters.try_emplace(localQuantumRoutine.value(), LocalRoutineRegisters{});
                }
                lastAvailable =this->routinesRegisters[localQuantumRoutine.value()].allocateQRegistry();
                break;
            case assembly::R:
                if (!this->routinesRegisters.contains(localQuantumRoutine.value())) {
                    this->routinesRegisters.try_emplace(localQuantumRoutine.value(), LocalRoutineRegisters{});
                }
                lastAvailable =this->routinesRegisters[localQuantumRoutine.value()].allocateRRegistry();
                break;
        }
        return lastAvailable;
    }

    uint8_t iQoalaContext::allocateClassicalSocketForRemote(const std::string &remoteID) {
        // If there is already a classical socket for this remote, simply return it:
        if (this->classicalSocketIDs.contains(remoteID)) {
            return this->classicalSocketIDs.at(remoteID);
        }
        const uint32_t newSocketID = this->classicalSocketIDs.size();
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
        const uint32_t newSocketID = this->eprsSocketIDs.size();
        const auto result = this->eprsSocketIDs.try_emplace(remoteID, newSocketID);
        (void) result;
        assert(result.second && "Attempting to map a remote name that is already mapped");
        return newSocketID;
    }

    void iQoalaContext::markOperationAsVisited(Operation *operation) {
        this->visitedOps.insert(operation);
    }

    bool iQoalaContext::isOperationVisited(const Operation *operation) const {
        return this->visitedOps.contains(operation);
    }
};