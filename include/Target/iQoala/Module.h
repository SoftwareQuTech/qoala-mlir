#ifndef QOALA_MODULE_H
#define QOALA_MODULE_H

#include "Target/iQoala/iQoalaContext.h"
#include "Target/iQoala/iQoala.h"
#include "llvm/Support/raw_ostream.h"

namespace qoala::iqoala {
    class iQoalaModule : public helpers::PrintInterface{
    public:
        iQoalaModule(llvm::StringRef name, const iQoalaContext &context) : moduleName(name), iQoalaCtx(context) { }
        void print(mlir::raw_ostream &os) const override;

        iQoalaContext &getiQoalaContext() {
            return iQoalaCtx;
        }

        void addRemoteDeclaration(mlir::StringRef remoteName);
        void setModuleName(mlir::StringRef newModuleName);
        void addRoutine(QuantumRoutine &newRoutine);

    private:
        mlir::StringRef moduleName;

        /**
         * Struct that represents the program in iQoala format
         * It contains section objects that store all the "MC" objects of the qoala program
         */
        struct {
            MetaSection metaSection;
            HostSection hostSection;
            NetQASMSection netQASMSection;
            RequestSection requestSection;
        } iQoalaProgram;
        iQoalaContext iQoalaCtx;
    };
}

#endif //QOALA_MODULE_H
