
#include "mlir/Pass/Pass.h"

#include "Analysis/Helpers/Helpers.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h"

#define DEBUG_TYPE "fold-constants"

using namespace mlir;
using namespace qoala::dialects;

namespace qoala::analysis {
#define GEN_PASS_DEF_FOLDCONSTANTS
#include "Dialect/Helpers/HelperPasses.h.inc"

    class FoldConstantsPass : public impl::FoldConstantsBase<FoldConstantsPass> {
    public:
        using FoldConstantsBase::FoldConstantsBase;
        void runOnOperation() override;
    };

    void FoldConstantsPass::runOnOperation() {
        LLVM_DEBUG(llvm::dbgs() << "Running FoldConstantsPass...\n");
        if (Operation *op = this->getOperation(); failed(helpers::foldConstants(*op))) {
            signalPassFailure();
        }
    }
}