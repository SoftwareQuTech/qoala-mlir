#include "Target/iQoala/Module.h"
#include "Analysis/Helpers/Helpers.h"

#include "llvm/Support/raw_ostream.h"

using namespace mlir;

namespace qoala::iqoala {
    std::string tabStr = "    ";

    void iQoalaModule::print(raw_ostream &os) const {
        // Call the "print" functions in the sections of the executable
        os << this->iQoalaProgram.metaSection << "\n"
           << this->iQoalaProgram.hostSection << "\n"
           << this->iQoalaProgram.netQASMSection << "\n"
           << this->iQoalaProgram.requestSection << "\n";
    }

    void iQoalaModule::addRemoteDeclaration(StringRef remoteName) {
        std::string temp = remoteName.str();
        this->iQoalaProgram.metaSection.addRemote(temp);
    }

    void iQoalaModule::setModuleName(StringRef newModuleName) {
        std::string temp = newModuleName.str();
        this->moduleName = newModuleName;
        this->iQoalaProgram.metaSection.setName(temp);
    }

    void iQoalaModule::addRoutine(QuantumRoutine &newRoutine) {
        if (auto localRoutine = dyn_cast<LocalQuantumRoutine>(&newRoutine)) {
            this->iQoalaProgram.netQASMSection.addRoutine(*localRoutine);
            return;
        }
        if (auto remoteRoutine = dyn_cast<RequestQuantumRoutine>(&newRoutine)) {
            this->iQoalaProgram.requestSection.addRoutine(*remoteRoutine);
            return;
        }
    }
}
