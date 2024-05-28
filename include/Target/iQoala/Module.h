#ifndef QOALA_MODULE_H
#define QOALA_MODULE_H

#include "Target/iQoala/iQoalaContext.h"
#include "Target/iQoala/iQoala.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace qoala::iqoala {
    class Module {
    public:
        Module(StringRef name, iQoalaContext &context);
        void print(raw_ostream &os);

        iQoalaProgram &getiQoalaProgram() {
            return iQoalaProgram;
        }

        void addRemoteDeclaration(StringRef remoteName);
        void setModuleName(StringRef moduleName);

    private:
        StringRef moduleName;
        iQoalaProgram iQoalaProgram;
        iQoalaContext iQoalaCtx;
    };
}

#endif //QOALA_MODULE_H
