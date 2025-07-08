#include "mlir/Dialect/Func/IR/FuncOps.h"
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
        if (qmem::FuncOp func = getOperation(); failed(helpers::foldConstants(func))) {
            signalPassFailure();
        }
    }
}