#include "Target/iQoala/Module.h"
#include "Analysis/Helpers/Helpers.h"

#include "llvm/Support/raw_ostream.h"

using namespace mlir;

namespace qoala::iqoala {
    std::string tabStr = "    ";

    iQoalaContext *iQoalaModule::getiQoalaContext() const {
        return this->iQoalaCtx;
    }

    void iQoalaModule::print(raw_ostream &os) const {
        // Call the "print" functions in the sections of the executable
        os << this->iQoalaProgram.metaSection << "\n"
           << this->iQoalaProgram.hostSection << "\n"
           << this->iQoalaProgram.netQASMSection << "\n"
           << this->iQoalaProgram.requestSection << "\n";
    }

    void iQoalaModule::addRemoteDeclaration(const StringRef &remoteName,
        const bool classicalSocket, const bool eprsSocket) {
        const std::string temp = remoteName.str();
        this->iQoalaProgram.metaSection.addRemote(temp);
        // We will create the classical and EPRS socket for the remote if needed
        if (classicalSocket) {
            const uint8_t classicalSocketID = this->iQoalaCtx->allocateClassicalSocketForRemote(temp);
            this->iQoalaProgram.metaSection.addClassicalSocketForRemote(temp, classicalSocketID);
        }
        if (eprsSocket) {
            const uint8_t eprsSocketID = this->iQoalaCtx->allocateEPRSSocketForRemote(temp);
            this->iQoalaProgram.metaSection.addEPRSSocketForRemote(temp, eprsSocketID);
        }
    }

    void iQoalaModule::setModuleName(const StringRef newModuleName) {
        const std::string temp = newModuleName.str();
        this->moduleName = newModuleName;
        this->iQoalaProgram.metaSection.setName(temp);
    }

    void iQoalaModule::addRoutine(QuantumRoutine *newRoutine) {
        if (const auto localRoutine = dyn_cast<LocalQuantumRoutine>(newRoutine)) {
            this->iQoalaProgram.netQASMSection.addRoutine(localRoutine);
            return;
        }
        if (const auto remoteRoutine = dyn_cast<RequestQuantumRoutine>(newRoutine)) {
            this->iQoalaProgram.requestSection.addRoutine(remoteRoutine);
            return;
        }
    }
    QuantumRoutine *iQoalaModule::getRoutineByName(const StringRef name) const {
        if (const auto localRoutine = this->getLocalRoutineByName(name)) {
            return localRoutine;
        }
        return this->getRequestRoutineByName(name);
    }

    LocalQuantumRoutine *iQoalaModule::getLocalRoutineByName(const StringRef name) const {
        for (auto *localRoutine : this->iQoalaProgram.netQASMSection.getRoutines()) {
            if (localRoutine->getName() == name) {
                return localRoutine;
            }
        }
        return nullptr;
    }

    RequestQuantumRoutine *iQoalaModule::getRequestRoutineByName(const StringRef name) const {
        for (auto *localRoutine : this->iQoalaProgram.requestSection.getRoutines()) {
            if (localRoutine->getName() == name) {
                return localRoutine;
            }
        }
        return nullptr;
    }

    std::vector<LocalQuantumRoutine *> iQoalaModule::getLocalRoutines() const {
        return this->iQoalaProgram.netQASMSection.getRoutines();
    }

    uint8_t iQoalaModule::getClassicalSocketIDForRemote(const StringRef &remoteName) const {
        return this->iQoalaProgram.metaSection.getClassicalSocketForRemote(remoteName.str());
    }

    uint8_t iQoalaModule::getEPRSSocketIDForRemote(const StringRef &remoteName) const {
        return this->iQoalaProgram.metaSection.getEPRSSocketForRemote(remoteName.str());
    }

    std::string iQoalaModule::getParamNameForRemote(const std::string &remoteName) const {
        return this->iQoalaProgram.metaSection.getParamNameForRemote(remoteName);
    }

    Block *iQoalaModule::addHostBlock() {
        return this->iQoalaProgram.hostSection.createNewBlock();
    }
}
