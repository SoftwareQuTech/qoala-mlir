#ifndef QOALA_MODULE_H
#define QOALA_MODULE_H

#include "Target/iQoala/iQoalaContext.h"
#include "Target/iQoala/iQoala.h"

namespace qoala::iqoala {
    class iQoalaModule : public helpers::PrintInterface{
    public:
        iQoalaModule(const llvm::StringRef &name, iQoalaContext *context) : moduleName(name), iQoalaCtx(context) { }
        ~iQoalaModule() override = default;
        void print(mlir::raw_ostream &os) const override;

        [[nodiscard]]
        iQoalaContext *getiQoalaContext() const ;

        void setModuleName(mlir::StringRef newModuleName);
        void addRemoteDeclaration(const mlir::StringRef &remoteName, bool classicalSocket = true, bool eprsSocket = true);
        void addRoutine(QuantumRoutine *newRoutine);
        Block *addHostBlock();
        [[nodiscard]]
        QuantumRoutine *getRoutineByName(mlir::StringRef name) const;
        [[nodiscard]]
        LocalQuantumRoutine *getLocalRoutineByName(mlir::StringRef name) const;
        [[nodiscard]]
        RequestQuantumRoutine *getRequestRoutineByName(mlir::StringRef name) const;
        [[nodiscard]]
        std::vector<LocalQuantumRoutine *> getLocalRoutines() const;
        [[nodiscard]]
        uint8_t getClassicalSocketIDForRemote(const mlir::StringRef &remoteName) const;
        [[nodiscard]]
        uint8_t getEPRSSocketIDForRemote(const mlir::StringRef &remoteName) const;
        [[nodiscard]]
        std::string getParamNameForRemote(const std::string &remoteName) const;

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
        iQoalaContext *iQoalaCtx;
    };
}

#endif //QOALA_MODULE_H
