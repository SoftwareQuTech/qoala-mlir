#include "Dialect/QMem/Passes.h"
#include "Dialect/QMem/QMem.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Support/LLVM.h"
#include "Analysis/QMem/Conversion.h"
#include "llvm/Support/Debug.h"

using namespace mlir;
using namespace qoala::dialects;
using namespace qoala::analysis;

#define DEBUG_TYPE "functionize"

static bool qMemOpCanBeFunctionized(mlir::Operation *op) {
    // A list fo the QMem operation types we would like to "functionize"
    return llvm::isa<
            qmem::CnotOp,
            qmem::CrotXOp,
            qmem::CzOp,
            qmem::EprsMeasureOp,
            qmem::EprsOp,
            qmem::HadamardOp,
            qmem::InitOp,
            qmem::MeasureOp,
            qmem::QAllocOp,
            // Recv/Send Ints/Floats can stay in the main body
//            qmem::RecvFloatsOp,
//            qmem::RecvIntsOp,
//            qmem::SendFloatsOp,
//            qmem::SendIntsOp,
            qmem::RotateXOp,
            qmem::RotateYOp,
            qmem::RotateZOp
            // We don't want to functionize "Remotes", "Funcs" nor "Returns"
//            qmem::RemoteOp,
//            qmem::FuncOp,
//            qmem::ReturnOp,
    >(op);
}

static functionize::BucketsTy simpleOpClassifier(ModuleOp *module) {
    std::vector<Region *> foundRegion;
    module->walk([&](qmem::FuncOp mainFunc) {
        foundRegion.push_back(&mainFunc.getBody());
    });

    std::vector<std::vector<Operation *>> result;
    // We simply put one quantum function inside its own group
    for (Operation &op : foundRegion[0]->getOps()) {
        if (qMemOpCanBeFunctionized(&op)) {
            std::vector<Operation *> intermediate;
            intermediate.push_back(&op);
            result.push_back(intermediate);
        }
    }
    return result;
}

namespace qoala::analysis {
#define GEN_PASS_DEF_QMEMSIMPLEFUNCTIONIZE
#include "Dialect/QMem/Passes.h.inc"

    class QMemSimpleFunctionizePass : public impl::QMemSimpleFunctionizeBase<QMemSimpleFunctionizePass> {
    public:
        using QMemSimpleFunctionizeBase::QMemSimpleFunctionizeBase;
        void runOnOperation() override;
    };

    void QMemSimpleFunctionizePass::runOnOperation() {
        ModuleOp module = dyn_cast<ModuleOp>(getOperation());
        assert(module); // We expect the cast to succeed
        LLVM_DEBUG(llvm::dbgs() << "Functionzing module\n");
        functionize::functionizeModule(module, simpleOpClassifier);
    }
} /* namespace qoala::analysis */