#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "mlir/IR/Operation.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;
using namespace qoala::dialects;

namespace qoala::analysis {

    QoalaHostQMemoryEfficiency::QoalaHostQMemoryEfficiency(Operation *op) {
        for (Region &reg: op->getRegions()) {
            for (Block &block: reg.getBlocks()) {
                for (Operation &func: block.getOperations()) {
                    if (auto mainFunc = dyn_cast<qoalahost::MainFuncOp>(&func)) {
                        ++logicalQubits;
                        ++physicalQubits;
                        ++logicalQubits;
                    }
                }
            }
        }
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
