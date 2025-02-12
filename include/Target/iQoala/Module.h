#ifndef QOALA_MODULE_H
#define QOALA_MODULE_H

#include "Target/iQoala/iQoalaContext.h"
#include "Target/iQoala/iQoala.h"
#include "llvm/Support/raw_ostream.h"

namespace qoala::iqoala {
    class iQoalaModule {
    public:
        iQoalaModule(llvm::StringRef name, const iQoalaContext &context) : moduleName(name),iQoalaCtx(context) { }
        void print(mlir::raw_ostream &os) const;

        iQoalaProgram &getiQoalaProgram() {
            return iQoalaProgram;
        }
        iQoalaContext &getiQoalaContext() {
            return iQoalaCtx;
        }

        void addRemoteDeclaration(mlir::StringRef remoteName);
        void setModuleName(mlir::StringRef newModuleName);
        void addRoutine(QuantumRoutine &newLocalRoutine);

    private:
        mlir::StringRef moduleName;
        iQoalaProgram iQoalaProgram;
        iQoalaContext iQoalaCtx;
    };
}

#endif //QOALA_MODULE_H
