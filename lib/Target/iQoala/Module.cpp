#include "Target/iQoala/Module.h"

#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace qoala::iqoala {
    Module::Module(llvm::StringRef name, qoala::iqoala::iQoalaContext &context) :
    moduleName(name), iQoalaProgram(), iQoalaCtx(context) { }

    void Module::print(raw_ostream &os) {
        os << this->iQoalaProgram;
    }

    void Module::addRemoteDeclaration(llvm::StringRef remoteName) {
        std::string temp = remoteName.str();
        this->iQoalaProgram.addRemoteDeclaration(temp);
    }

    void Module::setModuleName(llvm::StringRef moduleName) {
        std::string temp = moduleName.str();
        this->iQoalaProgram.setProgramName(temp);
    }
}
