#include "Target/iQoala/Module.h"

#include "llvm/Support/raw_ostream.h"

using namespace mlir;

namespace qoala::iqoala {
    void iQoalaModule::print(raw_ostream &os) const {
        os << this->iQoalaProgram;
    }

    void iQoalaModule::addRemoteDeclaration(StringRef remoteName) {
        std::string temp = remoteName.str();
        this->iQoalaProgram.addRemoteDeclaration(temp);
    }

    void iQoalaModule::setModuleName(StringRef newModuleName) {
        std::string temp = newModuleName.str();
        this->moduleName = newModuleName;
        this->iQoalaProgram.setProgramName(temp);
    }

    void iQoalaModule::addRoutine(QuantumRoutine &newLocalRoutine) {
        this->iQoalaProgram.addRoutine(newLocalRoutine);
    }
}
