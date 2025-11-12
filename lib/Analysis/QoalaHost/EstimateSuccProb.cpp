#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "llvm/ADT/TypeSwitch.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "mlir/IR/Operation.h"

#define DEBUG_TYPE "qoalahost-esp"

using namespace mlir;
using namespace qoala::dialects;

namespace qoala::analysis::esp {

    QoalaHostEstimateSuccProb::QoalaHostEstimateSuccProb(Operation *op) {
        // Bla bla
        auto mainFunc = dyn_cast_or_null<qoalahost::MainFuncOp>(*op);
        assert(mainFunc && "No main func? This is embarrassing...");

        const auto callOps = mainFunc.getOps<qoalahost::CallOp>();

        const float gateErr = 0.1;
        const uint32_t t1 = 1;
        const uint32_t t2 = 1;

        for (auto callOp : callOps) {
            auto callee = callOp.getCalleeOperation<FunctionOpInterface>();
            callee.walk([&](Operation *opInCallee) {
                llvm::TypeSwitch<Operation *>(opInCallee)
                        .Case([&](const netqasm::QAllocOp qallocOp) {
                            LLVM_DEBUG(llvm::dbgs() << "Found QAllocOp: " << qallocOp << "\n");
                        })
                        .Case([&](const netqasm::ifaces::MeasOp measureOp) {
                            LLVM_DEBUG(llvm::dbgs() << "Found MeasureOp: " << measureOp << "\n");
                        });
            });
        }
    }

} // namespace qoala::analysis::esp
