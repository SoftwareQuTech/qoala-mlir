#include "Analysis/Helpers/Helpers.h"
#include "Analysis/QMem/Conversion.h"
#include "Conversion/Helpers/Helpers.h"
#include "Dialect/Helpers/MIRToLIRHelperPasses.h"
#include "llvm/Support/Debug.h"
#include "mlir/IR/BuiltinOps.h"

using namespace mlir;
using namespace qoala::dialects;

#define DEBUG_TYPE "simple-functionize"

namespace qoala::analysis {
#define GEN_PASS_DEF_SIMPLEFUNCTIONIZE
#include "Dialect/Helpers/HelperPasses.h.inc"

    class QMemSimpleFunctionizePass : public impl::SimpleFunctionizeBase<QMemSimpleFunctionizePass> {
    public:
        using SimpleFunctionizeBase::SimpleFunctionizeBase;
        void runOnOperation() override;
    };

    void QMemSimpleFunctionizePass::runOnOperation() {
        ModuleOp module = this->getOperation();
        LLVM_DEBUG(llvm::dbgs() << "Functionzing module\n");
        functionize::functionizeModule(module, functionize::simpleOpClassifier, 0);
        // Correct the positions of the remote and builtin declaration
        module.walk([&](func::FuncOp funcDecl) {
            if (funcDecl.getSymName() != helpers::angle::angleConversionFunctionName) {
                WalkResult::advance();
            } else {
                helpers::moveOperationToTop(module, funcDecl);
            }
        });
        module.walk([&](const qmem::RemoteOp remote) { helpers::moveOperationToTop(module, remote); });
    }
} /* namespace qoala::analysis */
