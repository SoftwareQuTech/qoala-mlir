#include "Dialect/QMem/Passes.h"
#include "Dialect/QMem/QMem.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"

namespace mlir {
#define GEN_PASS_DEF_QMEMFACTORIZEREMOTEDECLS
#include "Dialect/QMem/Passes.h.inc"
} // namespace mlir

using namespace mlir;
using namespace qoala::dialects::qmem;

namespace {

class QMemFactorizeRemoteDeclsPass : public impl::QMemFactorizeRemoteDeclsBase<QMemFactorizeRemoteDeclsPass> {
    void runOnOperation() override;
};

} // namespace qoala::analysis

void QMemFactorizeRemoteDeclsPass::runOnOperation() {
    Operation *operation = getOperation();
}

std::unique_ptr<Pass> mlir::createQMemFactorizeRemoteDeclsPass() {
    return std::make_unique<QMemFactorizeRemoteDeclsPass>();
}
