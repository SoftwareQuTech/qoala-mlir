#ifndef QOALA_MODULE_H
#define QOALA_MODULE_H

#include "Target/iQoala/iQoalaContext.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace qoala::iqoala {
    class Module {
    private:
        StringRef moduleName;
        iQoalaContext iQoalaCtx;
    public:
        Module(StringRef name, iQoalaContext &context);
        ~Module() = default;
        void print(raw_ostream &os);

        raw_ostream &operator<<(raw_ostream &os) {
            this->print(os);
            return os;
        }
    };
}

#endif //QOALA_MODULE_H
