#include "Analysis/QMem/Conversion.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "float-flat"

using namespace mlir;

namespace qoala::analysis::flattening {
    void flattenFloatInstances(ModuleOp &module, bool (*operationUsesF32Angle)(Operation *)) {
        llvm::SetVector<Operation *> f32Operations;
        Builder builder(module->getContext());
        Type f32Type = builder.getF32Type();

        module.walk([&](Operation *operation) {
            for (Type resType : operation->getResultTypes()) {
                if (resType == f32Type) {
                    f32Operations.insert(operation);
                }
            }
        });

        for (Operation *f32Op : f32Operations) {
            LLVM_DEBUG(llvm::dbgs() << "Generating Op: " << *f32Op << "\n");
            LLVM_DEBUG(llvm::dbgs() << "Users:\n");
            for (Operation *user: f32Op->getUsers()) {
                LLVM_DEBUG(llvm::dbgs() << "* User: " << *user << "\n");

                if (operationUsesF32Angle(f32Op)) {
                    // TODO - insert a call to the builtin
                    // TODO - create the new "intermediate" operation (using the respective arguments and i32 angle)
                    // TODO - replace old operation with the new one
                }
            }
        }
    }
}