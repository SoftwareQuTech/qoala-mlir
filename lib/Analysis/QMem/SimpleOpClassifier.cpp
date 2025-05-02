#include "Analysis/QMem/Conversion.h"

using namespace mlir;

namespace qoala::analysis::functionize {
    // Helper function to determine if the operation can be functionized, used by "simpleOpClassifier"
    static bool qMemOpCanBeSimplyFunctionized(const Operation &op) {
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
        // Some operations need to be grouped as a pair.
        // * qallloc operations are considered to only "declare" a qubit
        // * We are not "defining" the qubit until we find a respective "init" or "eprs" operation
        // * In this sense, qalloc+init/eprs/eprs_measure need to be *grouped together*, despite this simple
        //   functionize algorithm creates 1 group per operation
        // To support this, we will use a map to keep track of the values yield by qalloc ops.
        // This map will always contain the "declared but not yet defined" qubits.
        DenseMap<Value, dialects::qmem::QAllocOp> declaredQubits;

        std::vector<QuantumOpsGroupTy> opsGroups;
        for (Operation &op : mainFunction.getOps()) {
            if (qMemOpCanBeSimplyFunctionized(op)) {
                if (auto qallocOp = llvm::dyn_cast<dialects::qmem::QAllocOp>(op)) {
                    const auto result = declaredQubits.try_emplace(qallocOp.getQ(), qallocOp);
                    (void) result;
                    assert(result.second && "Simple functionize: trying to map a qalloc operation that was already mapped");
                    continue;
                }
                if (auto initOp = llvm::dyn_cast<dialects::qmem::InitOp>(op)) {
                    assert(declaredQubits.contains(initOp.getQ()) && "Simple functionize: trying to group an init operation "
                                                                     "whose qubit has not been declared (qalloc)");
                    QuantumOpsGroupTy currentOpsGroup;
                    currentOpsGroup.push_back(declaredQubits[initOp.getQ()].getOperation());
                    currentOpsGroup.push_back(initOp.getOperation());
                    opsGroups.push_back(currentOpsGroup);
                    declaredQubits.erase(initOp.getQ());
                    continue;
                }
                if (auto eprsOp = llvm::dyn_cast<dialects::qmem::EprsOp>(op)) {
                    assert(declaredQubits.contains(eprsOp.getQ()) && "Simple functionize: trying to group an eprs operation "
                                                                     "whose qubit has not been declared (qalloc)");
                    QuantumOpsGroupTy currentOpsGroup;
                    currentOpsGroup.push_back(declaredQubits[eprsOp.getQ()].getOperation());
                    currentOpsGroup.push_back(eprsOp.getOperation());
                    opsGroups.push_back(currentOpsGroup);
                    declaredQubits.erase(eprsOp.getQ());
                    continue;
                }
                if (auto eprsOp = llvm::dyn_cast<dialects::qmem::EprsMeasureOp>(op)) {
                    assert(declaredQubits.contains(eprsOp.getQ()) && "Simple functionize: trying to group an eprs_measure operation "
                                                                     "whose qubit has not been declared (qalloc)");
                    QuantumOpsGroupTy currentOpsGroup;
                    currentOpsGroup.push_back(declaredQubits[eprsOp.getQ()].getOperation());
                    currentOpsGroup.push_back(eprsOp.getOperation());
                    opsGroups.push_back(currentOpsGroup);
                    declaredQubits.erase(eprsOp.getQ());
                    continue;
                }
                // Any other operation will go in its own group
                QuantumOpsGroupTy currentOpsGroup;
                currentOpsGroup.push_back(&op);
                opsGroups.push_back(currentOpsGroup);
            }
        }
        return opsGroups;
    }
}
