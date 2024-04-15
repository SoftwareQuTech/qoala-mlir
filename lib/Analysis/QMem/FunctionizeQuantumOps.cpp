#include "Dialect/QMem/Passes.h"
#include "Dialect/QMem/QMem.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"

namespace mlir {
#define GEN_PASS_DEF_QMEMSIMPLEFUNCTIONIZE
#include "Dialect/QMem/Passes.h.inc"
} // namespace mlir

using namespace mlir;
using namespace qoala::dialects;

namespace {

class QMemSimpleFunctionizePass : public impl::QMemSimpleFunctionizeBase<QMemSimpleFunctionizePass> {
    void runOnOperation() override;
};

} // namespace

void QMemSimpleFunctionizePass::runOnOperation() {
    ModuleOp module = dyn_cast<ModuleOp>(getOperation());
    OpBuilder builder(module->getContext());

    DenseSet<qmem::RemoteOp> uniqueRemotes;
    DenseMap<StringAttr, qmem::RemoteOp> filteredDecls;
    DenseSet<qmem::RemoteOp> toBeErased;
    module.walk([&](qmem::RemoteOp remoteDecl) {
        // We try to insert the remote in the set of unique declarations
        // If "f" was already defined, then firstDecl points to the first declaration
        // and "inserted" is false
        auto [firstDecl, inserted] = uniqueRemotes.insert(remoteDecl);
        // We leave a mark abot the first declaration. This operation is "void"
        // of the remote was already declared before
        filteredDecls[remoteDecl.getSymNameAttr()] = *firstDecl;
        // Since we are "moving" the declaration to a new place, we need to delete the original one
        toBeErased.insert(remoteDecl);
    });
    // We create the new, unique declarations on top of the visibility
    Location top = module.getBodyRegion().front().getOperations().front().getLoc();
    for (auto uniqueRemote : uniqueRemotes) {
        // This creates the remote declaration with a given location,
        // *but it does not insert it in the IR*
        auto newRemoteDecl =
                builder.create<qmem::RemoteOp>(top, uniqueRemote.getSymNameAttr(),
                                               uniqueRemote.getSymVisibilityAttr());
        // We insert the new remote declaration at the front position of the first block of the module
        module.getBodyRegion().getBlocks().front().push_front(newRemoteDecl);
    }
    // We update the references on all the operations that use th remote
    // Since the name of the remote did not change, this might not be needed!
    // Repeat for each type of Operation that uses the remote reference
    /*
    module.walk([&](qmem::EprsMeasureOp sendOp){
        qmem::RemoteOp reference = filteredDecls[sendOp.getRemoteAttr().getAttr()];
        sendOp.setRemote(reference.getSymName());
    });
    */
    // We finally delete the old declarations
    for (auto remoteToDelete: toBeErased) {
        remoteToDelete.erase();
    }
}

std::unique_ptr<Pass> mlir::createQMemSimpleFunctionize() {
    return std::make_unique<QMemSimpleFunctionizePass>();
}
