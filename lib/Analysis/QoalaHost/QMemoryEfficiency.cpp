#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/TypeSwitch.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/IR/Operation.h"

#define DEBUG_TYPE "qoalahost-qmemory-efficiency"

using namespace mlir;
using namespace qoala::dialects;

namespace qoala::analysis::qmemeff {

    // Process qalloc and measure operations inside a callee function,
    // updating virtualQubits, physicalQubits and measured buffer accordingly.
    static void processCalleeQMemOps(FunctionOpInterface callee, uint32_t &virtualQubits, uint32_t &physicalQubits,
                                     uint32_t &measured) {
        callee.walk([&](Operation *opInCallee) {
            llvm::TypeSwitch<Operation *>(opInCallee)
                    .Case([&](const netqasm::QAllocOp qallocOp) {
                        LLVM_DEBUG(llvm::dbgs() << "Found QAllocOp: " << qallocOp << "\n");
                        ++virtualQubits;
                        if (measured == 0) {
                            ++physicalQubits;
                        } else {
                            --measured;
                        }
                    })
                    .Case([&](const netqasm::ifaces::MeasOp measureOp) {
                        LLVM_DEBUG(llvm::dbgs() << "Found MeasureOp: " << measureOp << "\n");
                        ++measured;
                    });
        });
    }

    // Struct to track counts during branch evaluation.
    struct BranchCounts {
        uint32_t virtualQubits;
        uint32_t physicalQubits;
        uint32_t measured;

        void dumpInto(uint32_t &virt, uint32_t &phys, uint32_t &meas) const {
            virt = virtualQubits;
            phys = physicalQubits;
            meas = measured;
        }
    };

    // Traverse CFG and count qalloc/measure ops.
    static void traverseCFGAndCountQMem(mlir::Block *block, llvm::DenseSet<mlir::Block *> &visited,
                                        uint32_t &virtualQubits, uint32_t &physicalQubits, uint32_t &measured) {
        if (visited.contains(block)) {
            return;
        }
        visited.insert(block);

        LLVM_DEBUG(llvm::dbgs() << "Visiting block.\n");

        // Process all operations in current block except CF terminators.
        for (auto &op : block->getOperations()) {
            if (isa<cf::CondBranchOp, cf::BranchOp>(op)) {
                break;
            }

            if (auto callOp = dyn_cast<qoalahost::CallOp>(&op)) {
                auto callee = callOp.getCalleeOperation<FunctionOpInterface>();
                processCalleeQMemOps(callee, virtualQubits, physicalQubits, measured);
            }
        }

        // Handle branching via terminator.
        auto terminator = block->getTerminator();
        if (auto condBr = dyn_cast<cf::CondBranchOp>(terminator)) {
            LLVM_DEBUG(llvm::dbgs() << "Evaluating conditional branch.\n");
            // Capture initial counts
            BranchCounts initialCounts{virtualQubits, physicalQubits, measured};

            // Analyze true branch
            llvm::DenseSet<Block *> visitedTrue = visited;
            traverseCFGAndCountQMem(condBr.getTrueDest(), visitedTrue, virtualQubits, physicalQubits, measured);
            BranchCounts trueCounts{virtualQubits, physicalQubits, measured};
            LLVM_DEBUG(llvm::dbgs() << "TRUE branch evaluated: physicalQubits=" << trueCounts.physicalQubits << ".\n");

            // Restore initial counts and analyze false branch
            initialCounts.dumpInto(virtualQubits, physicalQubits, measured);
            llvm::DenseSet<Block *> visitedFalse = visited;
            traverseCFGAndCountQMem(condBr.getFalseDest(), visitedFalse, virtualQubits, physicalQubits, measured);
            BranchCounts falseCounts{virtualQubits, physicalQubits, measured};
            LLVM_DEBUG(llvm::dbgs() << "FALSE branch evaluated: physicalQubits=" << falseCounts.physicalQubits
                                    << ".\n");

            // Pick worst case: highest physicalQubits -> worst efficiency.
            if (trueCounts.physicalQubits >= falseCounts.physicalQubits) {
                trueCounts.dumpInto(virtualQubits, physicalQubits, measured);
                visited = visitedTrue;
            } else {
                // Variables already contain false branch counts
                visited = visitedFalse;
            }
        } else if (auto br = dyn_cast<cf::BranchOp>(terminator)) {
            traverseCFGAndCountQMem(br.getDest(), visited, virtualQubits, physicalQubits, measured);
        } else if (auto *nextBlock = block->getNextNode()) {
            traverseCFGAndCountQMem(nextBlock, visited, virtualQubits, physicalQubits, measured);
        }
    }

    QoalaHostQMemoryEfficiency::QoalaHostQMemoryEfficiency(Operation *op) {
        // Compute quantum memory efficiency as 1-(physicalQubits/virtualQubits), the closer to 1 the better.
        // Virtual qubits correspond to the number of qalloc ops present in a program,
        // physical qubits instead refers to the actual qubits that will be used by
        // the QNPU during program execution. These two numbers may differ as each time
        // a qubit is measured, its memory location is freed and can be reused by a subsequent qalloc op.
        // By tracking the order of qalloc and measurement ops, one can compute how many physical qubits
        // will be active at most at the same time during a program execution.
        // For conditional branches, both paths are evaluated and the worst case
        // (highest physical qubit count) is selected.

        auto mainFunc = dyn_cast_or_null<qoalahost::MainFuncOp>(*op);
        assert(mainFunc && "No main func? This is embarrassing...");

        uint32_t measured = 0;
        llvm::DenseSet<Block *> visited;
        traverseCFGAndCountQMem(&mainFunc.front(), visited, virtualQubits, physicalQubits, measured);
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
