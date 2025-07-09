#include "mlir/IR/BuiltinOps.h"
#include "Analysis/QMem/Conversion.h"
#include "Analysis/Helpers/Helpers.h"
#include "Conversion/Helpers/Helpers.h"
#include "Dialect/Helpers/MIRToLIRHelperPasses.h"
#include "llvm/Support/Debug.h"

using namespace mlir;
using namespace qoala::dialects;

#define DEBUG_TYPE "functionize"

namespace qoala::analysis {
#define GEN_PASS_DEF_FUNCTIONIZEQUANTUMOPS
#include "Dialect/Helpers/HelperPasses.h.inc"

    class FunctionizeQuantumOpsPass : public impl::FunctionizeQuantumOpsBase<FunctionizeQuantumOpsPass> {
    public:
        using FunctionizeQuantumOpsBase::FunctionizeQuantumOpsBase;
        void runOnOperation() override;
    };

    void FunctionizeQuantumOpsPass::runOnOperation() {
        ModuleOp module = this->getOperation();
        LLVM_DEBUG(llvm::dbgs() << "Functionzing module\n");

        // Select the right functionization algorithm depending on the options
        functionize::ClassifierFnTy classifier;
        if (this->useSimpleFunctionize) {
            LLVM_DEBUG(llvm::dbgs() << "WARNING - Using simple functionization\n");
            classifier = functionize::simpleOpClassifier;
        } else {
            LLVM_DEBUG(llvm::dbgs() << "Using normal functionzation\n");
            classifier = functionize::functionizeOpClassifier;
        }

        // Apply the functionization
        functionize::functionizeModule(module, classifier, this->maxOpsPerGroup);

        // Correct the positions of the remote and builtin declaration
        module.walk([&](func::FuncOp funcDecl) {
            if (funcDecl.getSymName() == helpers::angle::angleConversionFunctionName) {
                helpers::moveOperationToTop(module, funcDecl);
            }
            WalkResult::advance();
        });
        module.walk([&](const qmem::RemoteOp remote) {
            helpers::moveOperationToTop(module, remote);
        });
    }
} /* namespace qoala::analysis */