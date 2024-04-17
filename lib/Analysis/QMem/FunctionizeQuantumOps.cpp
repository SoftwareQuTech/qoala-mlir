#include "Dialect/QMem/Passes.h"
#include "Dialect/QMem/QMem.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"

#include <format>

#include "llvm/Support/Debug.h"

namespace mlir {
#define GEN_PASS_DEF_QMEMSIMPLEFUNCTIONIZE
#include "Dialect/QMem/Passes.h.inc"
} // namespace mlir

using namespace mlir;
using namespace qoala::dialects;

class QMemSimpleFunctionizePass : public impl::QMemSimpleFunctionizeBase<QMemSimpleFunctionizePass> {
    void runOnOperation() override;
};

static bool isQuantumOp(mlir::Operation &op) {
    return llvm::isa<
            qmem::CnotOp,
            qmem::CrotXOp,
            qmem::CzOp,
            qmem::EprsMeasureOp,
            qmem::EprsOp,
//            qmem::FuncOp,
            qmem::HadamardOp,
            qmem::InitOp,
            qmem::MeasureOp,
            qmem::QAllocOp,
            qmem::RecvFloatsOp,
            qmem::RecvIntsOp,
//            qmem::RemoteOp,
//            qmem::ReturnOp,
            qmem::RotateXOp,
            qmem::RotateYOp,
            qmem::RotateZOp,
            qmem::SendFloatsOp,
            qmem::SendIntsOp
    >(op);
}

static int identifier = 0;

void QMemSimpleFunctionizePass::runOnOperation() {
    ModuleOp module = dyn_cast<ModuleOp>(getOperation());
    Location topLocation = module.getBodyRegion().front().getOperations().front().getLoc();
    assert(module);

    SmallVector<Operation *, 100> quantumOps;
    // Discover and mark the quantum operations
    module.walk([&](Operation *operation) {
        if (!isQuantumOp(*operation)) {
            return;
        }
        quantumOps.push_back(operation);
    });

    for (auto quantumOp : quantumOps) {
        OpBuilder opBuilder(module.getBodyRegion());
        llvm::dbgs() << "*******************\n";
        llvm::dbgs() << "old quantumOp : " << *quantumOp << "\n";

        // Create a new function at the top level, using the types of the original operation
        FunctionType funType = opBuilder.getFunctionType(
                quantumOp->getOperandTypes(),
                quantumOp->getResultTypes()
                );
        // Format the name of the new function
        std::string funcName = std::format("test{}", identifier++);
        // Create the new function, and attach a block to it
        auto newFunc = opBuilder.create<func::FuncOp>(topLocation, funcName, funType);
        Block *newBlock = newFunc.addEntryBlock();
        {
            // Create a new operation of the same type that the original one, that uses
            // the function arguments instead of dialect results of other ops
            // Helper - cast the quantum operation to the "SimpleCloneInterface" type, so
            // it is possible to invoke "simpleClone" on that operation
            auto castedOldOp = dyn_cast<qmem::iface::SimpleCloneInterface>(quantumOp);
            assert(castedOldOp); // We expect that the cast will succeed

            // New operations (including the clone) should be attached to the block of the new function
            OpBuilder::InsertionGuard g(opBuilder);
            opBuilder.setInsertionPointToStart(newBlock);
            auto cloneOp = castedOldOp.simpleClone(opBuilder, newFunc.getLoc());
            cloneOp->setOperands(newBlock->getArguments());

            // Create the "return" for the new operation
            opBuilder.create<func::ReturnOp>(cloneOp->getLoc(), cloneOp->getResults());
            llvm::dbgs() << "new FuncOp : " << newFunc << "\n";
        }
        auto insertedSymbol = module.lookupSymbol<func::FuncOp>(newFunc.getNameAttr());
        llvm::dbgs() << "symbol = " << insertedSymbol << "\n";
        {
            // Create a "call" operation in place of the original operation
            OpBuilder::InsertionGuard g(opBuilder);
            opBuilder.setInsertionPointAfter(quantumOp);
            auto callOp = opBuilder.create<func::CallOp>(
                    quantumOp->getLoc(), insertedSymbol, /*args=*/ValueRange(quantumOp->getOperands())
            );
            llvm::dbgs() << "new CallOp : " << callOp << "\n";

            // Finally, replace the usages of the old operation with the result of the "call"
            // and delete the old operation
            quantumOp->replaceAllUsesWith(callOp);
            quantumOp->erase();
        }
        llvm::dbgs() << "-------------------\n";
    }
}

std::unique_ptr<Pass> mlir::createQMemSimpleFunctionize() {
    return std::make_unique<QMemSimpleFunctionizePass>();
}
