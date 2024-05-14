#include "Analysis/QMem/Conversion.h"
#include "llvm/Support/Debug.h"

namespace qoala::analysis::functionize {
    // Helper function to determine if the operation can be functionized, used by "simpleOpClassifier"
    static bool qMemOpCanBeSimplyFunctionized(Operation &op) {
        // A list fo the QMem operation types we would like to "functionize"
        return llvm::isa<
                dialects::qmem::CnotOp,
                dialects::qmem::CzOp,
                dialects::qmem::EprsMeasureOp,
                dialects::qmem::EprsOp,
                dialects::qmem::HadamardOp,
                dialects::qmem::InitOp,
                dialects::qmem::MeasureOp,
                dialects::qmem::QAllocOp,
                /* We do not want to functionize any (leftover) float rotation (if any)
                 * Since rotations use a f32 angle arg, they are lowered to intermediate rotations
                 * (so the f32 can be transformed into 2xi32), THEN to netqasm rotations
                 * This allows us to place the call to the builtin angle conversion function before functionizing
                dialects::qmem::RotateXOp,
                dialects::qmem::RotateYOp,
                dialects::qmem::RotateZOp,
                dialects::qmem::CrotXOp
                 */
                /* Instead, we need to functionize the "integer" version of the rotations */
                dialects::qmem::RotateXIntOp,
                dialects::qmem::RotateYIntOp,
                dialects::qmem::RotateZIntOp,
                dialects::qmem::CrotXIntOp
                /* Recv/Send Ints/Floats can stay in the main body
                dialects::qmem::RecvFloatsOp,
                dialects::qmem::RecvIntsOp,
                dialects::qmem::SendFloatsOp,
                dialects::qmem::SendIntsOp,
                 */
                /* We don't want to functionize "Remotes", "Funcs" nor "Returns"
                dialects::qmem::RemoteOp,
                dialects::qmem::FuncOp,
                dialects::qmem::ReturnOp
                */
        >(op);
    }

    std::vector<QuantumOpsGroupTy> simpleOpClassifier(dialects::qmem::FuncOp &mainFunction) {
        std::vector<QuantumOpsGroupTy> opsGroups;
        for (Operation &op : mainFunction.getOps()) {
            if (qMemOpCanBeSimplyFunctionized(op)) {
                QuantumOpsGroupTy currentOpsGroup;
                currentOpsGroup.push_back(&op);
                opsGroups.push_back(currentOpsGroup);
            }
        }
        return opsGroups;
    }
}
