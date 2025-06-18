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
        LLVM_DEBUG(llvm::dbgs() << "Running QoalaHostQMemoryEfficiencyPass\n");
        ModuleOp module = dyn_cast<ModuleOp>(*op);
        auto mainFuncs = module.getOps<qoalahost::MainFuncOp>();
        assert(!mainFuncs.empty() && "No main func? This is embarrassing...");
        qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();
        
        int measured = 0;

        mainFunc.walk([&](mlir::Operation *op) {
            if (auto callOp = dyn_cast<qoalahost::CallOp>(op)) {
                const Operation *callee = SymbolTable::lookupNearestSymbolFrom(callOp, callOp.getCalleeAttr());
                if (auto routine = dyn_cast<FunctionOpInterface>(callee)) {
                    routine.walk([&](mlir::Operation *calleeOp) {
                        if (calleeOp && isa<netqasm::QAllocOp>(calleeOp)) {
                            LLVM_DEBUG(llvm::dbgs() << "Found QAllocOp: " << *calleeOp << "\n");
                            ++logicalQubits;
                            if (measured == 0) {
                                ++physicalQubits;
                            } else {
                                --measured;
                            }
                        } else if (calleeOp && isa<netqasm::MeasureOp>(calleeOp)) {
                            LLVM_DEBUG(llvm::dbgs() << "Found MeasureOp: " << *calleeOp << "\n");
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
