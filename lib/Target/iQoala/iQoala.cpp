#include "Target/iQoala/iQoala.h"

namespace qoala::iqoala {

    std::string tabStr = "    ";

    raw_ostream &operator<<(raw_ostream &os, const PrintInterface &printable) {
        printable.print(os);
        return os;
    }

    void iQoalaProgram::print(raw_ostream &os) const {
        // Call the "print" functions in the sections of the executable
        os << this->metaSection << "\n" << this->hostSection << "\n" << this->netQASMSection << "\n";
    }

    void MetaSection::addRemote(std::string &remoteName) {
        std::string newStr(remoteName + "_id");
        this->globalParams.push_back(newStr);
        this->classicalSocketsMap.insert(std::pair{remoteName, 0});
        this->eprsSocketsMap.insert(std::pair{remoteName, 0});
    }

    void iQoalaProgram::addRemoteDeclaration(std::string &remoteName) {
        this->metaSection.addRemote(remoteName);
    }
}
