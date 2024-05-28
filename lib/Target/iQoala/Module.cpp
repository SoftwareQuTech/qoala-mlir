#include "Target/iQoala/Module.h"

#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace qoala::iqoala {
    Module::Module(llvm::StringRef name, qoala::iqoala::iQoalaContext &context) :
    moduleName(name) , iQoalaCtx(context) { }

    void Module::print(raw_ostream &os) {
        // TODO - Implement this print
    }
}