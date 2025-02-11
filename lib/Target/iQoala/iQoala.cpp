#include "Target/iQoala/iQoala.h"
#include "llvm/ADT/TypeSwitch.h"

namespace qoala::iqoala {

    std::string tabStr = "    ";

    raw_ostream &operator<<(raw_ostream &os, const PrintInterface &printable) {
        printable.print(os);
        return os;
    }

    void iQoalaProgram::print(raw_ostream &os) const {
        // Call the "print" functions in the sections of the executable
        os << this->metaSection << "\n"
           << this->hostSection << "\n"
           << this->netQASMSection << "\n"
           << this->requestSection << "\n";
    }

    void NetQASMSection::addRoutine(LocalQuantumRoutine &routine) {
        this->routines.push_back(routine);
    }

    void RequestSection::addRoutine(RequestQuantumRoutine &routine) {
        this->routines.push_back(routine);
    }

    void MetaSection::addRemote(const std::string &remoteName) {
        std::string newStr(remoteName);
        this->globalParams.push_back(newStr);
        this->classicalSocketsMap.insert(std::pair{remoteName, 0});
        this->eprsSocketsMap.insert(std::pair{remoteName, 0});
    }

    void MetaSection::setName(const std::string &programName) {
        this->name = programName;
    }

    void iQoalaProgram::addRemoteDeclaration(const std::string &remoteName) {
        this->metaSection.addRemote(remoteName);
    }

    void iQoalaProgram::setProgramName(const std::string &programName) {
        this->metaSection.setName(programName);
    }

    void iQoalaProgram::addRoutine(QuantumRoutine &routine) {
        if (auto localRoutine = static_cast<LocalQuantumRoutine *>(&routine)) {
            this->netQASMSection.addRoutine(*localRoutine);
            return;
        }
        if (auto requestRoutine = static_cast<RequestQuantumRoutine *>(&routine)) {
            this->requestSection.addRoutine(*requestRoutine);
            return;
        }
    }
}
