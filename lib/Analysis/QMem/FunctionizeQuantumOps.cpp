#include "Dialect/QMem/Passes.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Support/LLVM.h"
#include "Analysis/QMem/Conversion.h"
#include "llvm/Support/Debug.h"

using namespace mlir;
using namespace qoala::dialects;
using namespace qoala::analysis;
using namespace qoala::analysis::functionize;

#define DEBUG_TYPE "functionize"

namespace qoala::analysis {
#define GEN_PASS_DEF_QMEMSIMPLEFUNCTIONIZE
#include "Dialect/QMem/Passes.h.inc"

    class QMemSimpleFunctionizePass : public impl::QMemSimpleFunctionizeBase<QMemSimpleFunctionizePass> {
    public:
        using QMemSimpleFunctionizeBase::QMemSimpleFunctionizeBase;
        void runOnOperation() override;
    };

    void QMemSimpleFunctionizePass::runOnOperation() {
        ModuleOp module = this->getOperation();
        LLVM_DEBUG(llvm::dbgs() << "Functionzing module\n");
        functionizeModule(module, simpleOpClassifier);
    }
} /* namespace qoala::analysis */