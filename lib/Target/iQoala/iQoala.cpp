#include "Target/iQoala/iQoala.h"

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

    void MetaSection::addRemote(std::string &remoteName) {
        std::string newStr(remoteName);
        this->globalParams.push_back(newStr);
        this->classicalSocketsMap.insert(std::pair{remoteName, 0});
        this->eprsSocketsMap.insert(std::pair{remoteName, 0});
    }

    void MetaSection::setName(std::string &programName) {
        this->name = programName;
    }

    void iQoalaProgram::addRemoteDeclaration(std::string &remoteName) {
        this->metaSection.addRemote(remoteName);
    }

    void iQoalaProgram::setProgramName(std::string &programName) {
        this->metaSection.setName(programName);
    }
}
