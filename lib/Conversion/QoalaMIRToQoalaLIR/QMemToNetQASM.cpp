#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/Support/Debug.h"

#include "Conversion/Helpers/Helpers.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h"

namespace mlir {
#define GEN_PASS_DEF_LOWERQMEMTONETQASM
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h.inc"
} // namespace mlir

#define DEBUG_TYPE "qmem-to-netqasm"

namespace qoala::conversion {
    class LowerQMemToNetQASMPass : public mlir::impl::LowerQMemToNetQASMBase<LowerQMemToNetQASMPass> {
        void runOnOperation() override;
    };

    void LowerQMemToNetQASMPass::runOnOperation() {
        // TODO - To Implement
    }
}

std::unique_ptr<mlir::Pass> mlir::createLowerQMemToNetQASMPass() {
    return std::make_unique<qoala::conversion::LowerQMemToNetQASMPass>();
}
