#include "Dialect/QMem/Passes.h"
#include "Dialect/QMem/QMem.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Support/LLVM.h"
#include "Analysis/QMem/Functionize.h"

namespace mlir {
#define GEN_PASS_DEF_QMEMSIMPLEFUNCTIONIZE
#include "Dialect/QMem/Passes.h.inc"
} // namespace mlir

using namespace mlir;
using namespace qoala::dialects;
using namespace qoala::analysis;

namespace qoala::analysis {
    class QMemSimpleFunctionizePass : public impl::QMemSimpleFunctionizeBase<QMemSimpleFunctionizePass> {
    public:
        void runOnOperation() override;
    };
}

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
            qmem::RecvFloatsOp,
            qmem::RecvIntsOp,
            qmem::RotateXOp,
            qmem::RotateYOp,
            qmem::RotateZOp,
            qmem::SendFloatsOp,
            qmem::SendIntsOp
            // We don't want to functionize "Remotes", "Funcs" nor "Returns"
//            qmem::RemoteOp,
//            qmem::FuncOp,
//            qmem::ReturnOp,
    >(op);
}

void QMemSimpleFunctionizePass::runOnOperation() {
    ModuleOp module = dyn_cast<ModuleOp>(getOperation());
    assert(module); // We expect the cast to succeed
    functionizeModule(module, qMemOpCanBeFunctionized);
}

std::unique_ptr<Pass> mlir::createQMemSimpleFunctionize() {
    return std::make_unique<qoala::analysis::QMemSimpleFunctionizePass>();
}
