#include "Dialect/QMem/Passes.h"
#include "Dialect/QMem/QMem.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"

#include <format>

namespace mlir {
#define GEN_PASS_DEF_QMEMSIMPLEFUNCTIONIZE
#include "Dialect/QMem/Passes.h.inc"
} // namespace mlir

using namespace mlir;
using namespace qoala::dialects;

class QMemSimpleFunctionizePass : public impl::QMemSimpleFunctionizeBase<QMemSimpleFunctionizePass> {
    void runOnOperation() override;
    static func::FuncOp createNewFunctionWithOperations(
            OpBuilder *, StringRef, Location , ArrayRef<Operation *>);
};

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
            // We don't want to functionize "Remotes", "Returns" nor "Funcs"
//            qmem::RemoteOp,
//            qmem::ReturnOp,
//            qmem::FuncOp,
    >(op);
}

/**
 * Creates a "wrapper" function around the given list of operations.
 * WARNING: This function will not perform any type of Data nor Control Flow
 * analysis on the given operations, and it assumes that the data and control
 * flow of the given list is consistent.
 * Being this said, this function will create a `func::FuncOp` objects whose
 * arguments match the operand types of the first given operation, and whose
 * return type match the result types of the last given operation
 * @param opBuilder An `OpBuilder` object used to clone the given operations.
 * @param loc The location to use as an attribute for the new operations created
 * @param quantumOps An `ArrayRef` object containing the set of operations to wrap.
 * @return A `func::FuncOp` object, inserted at the insertion point of the given `OpBuilder`,
 *         whose body contains the given operations, and an extra return statement.
 */
func::FuncOp QMemSimpleFunctionizePass::createNewFunctionWithOperations(
        OpBuilder *opBuilder, StringRef funcName, Location loc, ArrayRef<Operation *>quantumOps) {
    // Create a new function at the top level, using the types of the original operation
    Operation *firstOperation = quantumOps.front();
    Operation *lastOperation = quantumOps.back();
    FunctionType funType = opBuilder->getFunctionType(
            firstOperation->getOperandTypes(),
            lastOperation->getResultTypes()
    );
    // Create the new function, and attach a block to it
    auto newFunc = opBuilder->create<func::FuncOp>(loc, funcName, funType);
    Block *newBlock = newFunc.addEntryBlock();
    {
        // Create a new operation of the same type that the original one, that uses
        // the function arguments instead of dialect results of other ops
        // Helper - cast the quantum operation to the "SimpleCloneInterface" type, so
        // it is possible to invoke "simpleClone" on that operation
        Operation *lastClonedOp;
        OpBuilder::InsertionGuard g(*opBuilder);
        for (Operation *quantumOp : quantumOps) {
            auto castedOldOp = dyn_cast<qmem::iface::SimpleCloneInterface>(quantumOp);
            assert(castedOldOp); // We expect that the cast will succeed

            // New operations (including the clone) should be attached to the block of the new function
            opBuilder->setInsertionPointToStart(newBlock);
            auto clonedOp = castedOldOp.simpleClone(*opBuilder, newFunc.getLoc());
            clonedOp->setOperands(newBlock->getArguments());
            lastClonedOp = clonedOp;
        }

        // Create the "return" for the new operation
        opBuilder->create<func::ReturnOp>(lastClonedOp->getLoc(), lastClonedOp->getResults());
    }
    return newFunc;
}

static int identifier = 0;

void QMemSimpleFunctionizePass::runOnOperation() {
    ModuleOp module = dyn_cast<ModuleOp>(getOperation());
    Location topLocation = module.getBodyRegion().front().getOperations().front().getLoc();
    assert(module);
    SmallVector<Operation *, 100> quantumOps;
    DenseMap<Operation *, func::FuncOp> mappedFunctions;

    // Discover and mark the quantum operations
    module.walk([&](Operation *operation) {
        if (qMemOpCanBeFunctionized(operation)) {
            quantumOps.push_back(operation);
        }
    });

    // We start building our new functions at the start of the body of the module
    OpBuilder opBuilder(module.getBodyRegion());

    for (Operation *quantumOp : quantumOps) {
        // Format the name of the new function
        std::string funcName = std::format("__qoala_wrapper{}", identifier++);
        // We create a new function that will contain the single operation in its body (+ a return statement)
        func::FuncOp newFunc = createNewFunctionWithOperations(
                &opBuilder, StringRef{funcName}, topLocation, ArrayRef{quantumOp}
                );
        auto insertedSymbol = module.lookupSymbol<func::FuncOp>(newFunc.getNameAttr());
        mappedFunctions.insert(std::pair{quantumOp, insertedSymbol});
    }
    for (Operation *quantumOp : quantumOps) {
        // Create a "call" operation in place of the original operation
        OpBuilder::InsertionGuard g(opBuilder);
        opBuilder.setInsertionPointAfter(quantumOp);
        auto callOp = opBuilder.create<func::CallOp>(
                quantumOp->getLoc(), mappedFunctions[quantumOp],
                /*args=*/ValueRange(quantumOp->getOperands())
        );

        // Finally, replace the usages of the old operation with the result of the "call"
        // and delete the old operation
        quantumOp->replaceAllUsesWith(callOp);
        quantumOp->erase();
    }
}

std::unique_ptr<Pass> mlir::createQMemSimpleFunctionize() {
    return std::make_unique<QMemSimpleFunctionizePass>();
}
