#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/Support/Debug.h"

#include "Conversion/Helpers/Helpers.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h"

namespace mlir {
#define GEN_PASS_DEF_LOWERQMEMTOQOALAHOST
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h.inc"
} // namespace mlir

#define DEBUG_TYPE "qmem-to-qoalahost"

namespace qoala::conversion {
    class LowerQMemToQoalaHostPass : public mlir::impl::LowerQMemToQoalaHostBase<LowerQMemToQoalaHostPass> {
        void runOnOperation() override;
    };

    void LowerQMemToQoalaHostPass::runOnOperation() {
        // TODO - To implement
    }
}

std::unique_ptr<mlir::Pass> mlir::createLowerQMemToQoalaHostPass() {
    return std::make_unique<qoala::conversion::LowerQMemToQoalaHostPass>();
}
