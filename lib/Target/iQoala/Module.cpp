#include "Target/iQoala/Module.h"

#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace qoala::iqoala {
    iQoalaModule::iQoalaModule(llvm::StringRef name, qoala::iqoala::iQoalaContext &context) :
    moduleName(name), iQoalaProgram(), iQoalaCtx(context) { }

    void iQoalaModule::print(raw_ostream &os) {
        os << this->iQoalaProgram;
    }

    void iQoalaModule::addRemoteDeclaration(llvm::StringRef remoteName) {
        std::string temp = remoteName.str();
        this->iQoalaProgram.addRemoteDeclaration(temp);
    }

    void iQoalaModule::setModuleName(llvm::StringRef newModuleName) {
        std::string temp = newModuleName.str();
        this->moduleName = newModuleName;
        this->iQoalaProgram.setProgramName(temp);
    }
}
