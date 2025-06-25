#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "mlir/IR/Operation.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "qoalahost-qmemory-efficiency"

using namespace mlir;
using namespace qoala::dialects;

namespace qoala::analysis {

    QoalaHostQMemoryEfficiency::QoalaHostQMemoryEfficiency(Operation *op) {
        // Compute quantum memory efficiency as 1-(virtualQubits/physicalQubits), the closer to 1 the better.
        // Virtual qubits correspond to the number of qalloc ops present in a program,
        // physical qubits instead refers to the actual qubits that will be used by 
        // the QNPU during programm exectuion. This two numbers may differ as each time 
        // a qubit is measured, its memory location is freed and can be reaused by a subsequent qalloc op.
        // By trakcing the order of qalloc and measurement ops, one can compute how many physical qubits 
        // will be active at most at the same time during a program execution.
        // Some programs may have efficiency 0, for example a program 
        // where only one qubit is allocated and measured. 
        LLVM_DEBUG(llvm::dbgs() << "Running QoalaHostQMemoryEfficiencyPass\n");
        ModuleOp module = dyn_cast<ModuleOp>(*op);
        // Locate main function
        auto mainFuncs = module.getOps<qoalahost::MainFuncOp>();
        assert(!mainFuncs.empty() && "No main func? This is embarrassing...");
        qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();
        
        // Measurements ops will increase a "buffer" of memory postions to be reused.
        int measured = 0;

        // Walk trough the blocks in the main function and focus on qoalahost call ops,
        // then walk trough each operation in the callee and search for qalloc and measure operations.

        // TODO Current implementation does not take into account epr request callbacks.
        // In the future, add support for epr request callbacks if needed.
        mainFunc.walk([&](mlir::Operation *op) {
            if (auto callOp = dyn_cast<qoalahost::CallOp>(op)) {
                const Operation *callee = SymbolTable::lookupNearestSymbolFrom(callOp, callOp.getCalleeAttr());
                if (auto routine = dyn_cast<FunctionOpInterface>(callee)) {
                    routine.walk([&](mlir::Operation *calleeOp) {
                        if (calleeOp && isa<netqasm::QAllocOp>(calleeOp)) {
                            LLVM_DEBUG(llvm::dbgs() << "Found QAllocOp: " << *calleeOp << "\n");
                            // Increase logical qubits for every qalloc op.
                            ++logicalQubits;
                            // Increase physical qubits only if the measured buffer equals 0.
                            if (measured == 0) {
                                ++physicalQubits;
                            } else {
                                // Remember to decrease the measure buffer.
                                --measured;
                            }
                        } else if (calleeOp && (isa<netqasm::MeasureOp>(calleeOp) || isa<netqasm::EprsMeasureOp>(calleeOp))) {
                            LLVM_DEBUG(llvm::dbgs() << "Found MeasureOp: " << *calleeOp << "\n");
                            // Each measure op will increase the measured buffer.
                            ++measured;
                        }
                    });
                } else {
                    llvm::errs() << "Callee is not a FunctionOpInterface!\n";
                }
            }
        });
    }

    float QoalaHostQMemoryEfficiency::getEfficiency() const {
        if (logicalQubits == 0) {
            llvm::errs() << "Warning: logicalQubits is zero; cannot compute efficiency.\n";
            return 0.0f;  // Or return 1.0f if you prefer to treat "no usage" as maximally efficient
        }

        float efficiency = 1.0f - static_cast<float>(physicalQubits) / logicalQubits;
        return efficiency;
    }
} // namespace qoala::analysis
