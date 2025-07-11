#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "mlir/IR/Operation.h"
#include "llvm/ADT/TypeSwitch.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "qoalahost-qmemory-efficiency"

using namespace mlir;
using namespace qoala::dialects;

namespace qoala::analysis::qmemeff {

    QoalaHostQMemoryEfficiency::QoalaHostQMemoryEfficiency(Operation *op) {
        // Compute quantum memory efficiency as 1-(virtualQubits/physicalQubits), the closer to 1 the better.
        // Virtual qubits correspond to the number of qalloc ops present in a program,
        // physical qubits instead refers to the actual qubits that will be used by 
        // the QNPU during program execution. This two numbers may differ as each time
        // a qubit is measured, its memory location is freed and can be reused by a subsequent qalloc op.
        // By tracking the order of qalloc and measurement ops, one can compute how many physical qubits
        // will be active at most at the same time during a program execution.
        // Some programs may have efficiency 0, for example a program 
        // where only one qubit is allocated and measured. 
        LLVM_DEBUG(llvm::dbgs() << "Running QoalaHostQMemoryEfficiencyPass\n");
        auto module = dyn_cast<ModuleOp>(*op);
        // Locate main function
        const auto mainFuncs = module.getOps<qoalahost::MainFuncOp>();
        assert(!mainFuncs.empty() && "No main func? This is embarrassing...");
        qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();

        // Measurements ops will increase a "buffer" of memory positions to be reused.
        int measured = 0;

        // Walk through the blocks in the main function and focus on qoalahost call ops,
        // then walk through each operation in the callee and search for qalloc and measure operations.

        // TODO Current implementation does not take into account epr request callbacks.
        // In the future, add support for epr request callbacks if needed. See issue qoala-kanban-board#89.
        mainFunc.walk([&](qoalahost::CallOp callOp) {
            if (auto callee = callOp.getCalleeOperation<FunctionOpInterface>()) {
                callee.walk([&] (Operation *opInCallee) {
                    llvm::TypeSwitch<Operation *>(opInCallee)
                        .Case([&](const netqasm::QAllocOp qallocOp) {
                            LLVM_DEBUG(llvm::dbgs() << "Found QAllocOp: " << qallocOp << "\n");
                            // Increase virtual qubits for every qalloc op.
                            ++virtualQubits;
                            // Increase physical qubits only if the measured buffer equals 0.
                            if (measured == 0) {
                                ++physicalQubits;
                            } else {
                                // Remember to decrease the measure buffer.
                                --measured;
                            }})
                        .Case([&](const netqasm::ifaces::MeasureOpIface measureOp) {
                            LLVM_DEBUG(llvm::dbgs() << "Found MeasureOp: " << measureOp << "\n");
                            // Each measure op will increase the measured buffer.
                            ++measured;
                        });
                });
            } else {
                llvm::errs() << "Callee is not a FunctionOpInterface!\n";
            }
        });
    }

    float QoalaHostQMemoryEfficiency::getEfficiency() const {
        if (virtualQubits == 0) {
            llvm::errs() << "Warning: virtualQubits is zero; cannot compute efficiency.\n";
            return 0.0f; // Or return 1.0f if you prefer to treat "no usage" as maximally efficient
        }

        const float efficiency = 1.0f - static_cast<float>(physicalQubits) / static_cast<float>(virtualQubits);
        return efficiency;
    }
} // namespace qoala::analysis::qmemeff
