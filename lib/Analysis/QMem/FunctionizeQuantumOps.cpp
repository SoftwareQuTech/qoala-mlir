#include "Dialect/QMem/Passes.h"
#include "Dialect/QMem/QMem.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Support/LLVM.h"
#include "llvm/Support/Debug.h"

#include <cstdio>
#include <set>

namespace mlir {
#define GEN_PASS_DEF_QMEMSIMPLEFUNCTIONIZE
#include "Dialect/QMem/Passes.h.inc"
} // namespace mlir

/* MLIR magic: using the LLVM_DEBUG macro + this define, allows to selectively enable the debug output
 * To enable the prints, simply invoke `opt` with `--debug-only=functionize-poc`, or any other string defined
 * as the "DEBUG_TYPE" macro */
#define DEBUG_TYPE "functionize-poc"

using namespace mlir;
using namespace qoala::dialects;

class QMemSimpleFunctionizePass : public impl::QMemSimpleFunctionizeBase<QMemSimpleFunctionizePass> {
private:
    struct ArgTypesAndReturns {
        std::vector<Type> argTypes;
        std::vector<Type> resultTypes;
        mlir::DenseMap<Operation *, std::set<uint32_t>> opResultsIndexes;
    };
    void runOnOperation() override;
    static func::FuncOp createNewFunctionWithOperations(
            OpBuilder *, StringRef, Location, llvm::SetVector<Operation *> &);
    static ArgTypesAndReturns computeArgTypesAndReturns(SetVector<Operation *> &);
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
            // We don't want to functionize "Remotes", "Funcs" nor "Returns"
//            qmem::RemoteOp,
//            qmem::FuncOp,
//            qmem::ReturnOp,
    >(op);
}

QMemSimpleFunctionizePass::ArgTypesAndReturns
QMemSimpleFunctionizePass::computeArgTypesAndReturns(SetVector<Operation *> &quantumOps) {
    // We need to compute a sort of "closure" in the quantumOps set.
    // Create a new function at the top level, using the types of the "orphan" usages of the given ops:
    // If an operation that appears on the "uses" is not in the quantumOps set, then it's an
    // "outsider", hence it's an argument of the quantumOps set.
    // If an operation that appears on the "operands" is not in the quantumOps set, then it's an
    // "outsider", hence it's a result of the quantumOps set.
    std::vector<Type> argTypes;
    std::vector<Type> resultTypes;

    // Additionally, the treatment with the results it's a bit complicated
    // Since we will clone the quantumOp, we need to save the _index_ of the result that is a
    // return. So later, when we clone the operation we construct the list of the actual result
    // values using the indexes per operation.
    mlir::DenseMap<Operation *, std::set<uint32_t>> opResultsIndexes;
    for (Operation *quantumOp : quantumOps) {
        auto opOperands = quantumOp->getOperands();
        for (Value opOperand: opOperands) {
            if (!quantumOps.contains(opOperand.getDefiningOp())) {
                argTypes.push_back(opOperand.getType());
            }
        }
        ResultRange opResults = quantumOp->getResults();
        for (OpResult opResult : opResults) {
            // A set to store all the indexes of the results that will be returned
            std::set<uint32_t> returnIndexesForOp;
            unsigned int resultIndex = opResult.getResultNumber();

            if (opResult.getUses().empty()) {
                // If there are no uses, we still need to return the "unused" result
                resultTypes.push_back(opResult.getType());
                returnIndexesForOp.insert(resultIndex);
            } else {
                // If any of the result usages is not in the quantumOps (and that index is not on
                // the already marked for return), then it needs to be returned
                for (auto &usage : opResult.getUses()) {
                    if (!quantumOps.contains(usage.getOwner()) && !returnIndexesForOp.contains(resultIndex)) {
                        resultTypes.push_back(opResult.getType());
                        returnIndexesForOp.insert(resultIndex);
                    }
                }
            }
            opResultsIndexes.insert(std::pair{quantumOp, returnIndexesForOp});
        }
    }
    return QMemSimpleFunctionizePass::ArgTypesAndReturns{argTypes, resultTypes, opResultsIndexes};
}

/**
 * Creates a "wrapper" function around the given list of operations.
 * WARNING: This function will not perform any type of Data nor Control Flow
 * analysis on the given operations, and it assumes that the data and control
 * flow of the given list is consistent.
 * @param opBuilder An `OpBuilder` object used to clone the given operations.
 * @param funcName The name of the function to be created.
 * @param loc The location to use as an attribute for the new operations created.
 * @param quantumOps An `ArrayRef` object containing the set of operations to wrap.
 * @return A `func::FuncOp` object, inserted at the insertion point of the given `OpBuilder`,
 *         whose body contains the given operations, and an extra return statement.
 */
func::FuncOp QMemSimpleFunctionizePass::createNewFunctionWithOperations(
        OpBuilder *opBuilder, StringRef funcName,
        Location loc, SetVector<Operation *> &quantumOps) {
    QMemSimpleFunctionizePass::ArgTypesAndReturns typesAndValues = computeArgTypesAndReturns(quantumOps);

    // Create the new function, and attach a block to it
    FunctionType funType = opBuilder->getFunctionType(typesAndValues.argTypes, typesAndValues.resultTypes);
    auto newFunc = opBuilder->create<func::FuncOp>(loc, funcName, funType);
    Block *newBlock = newFunc.addEntryBlock();

    LLVM_DEBUG(llvm::dbgs() << "************************\n");
    LLVM_DEBUG(llvm::dbgs() << "func type: " << funType << "\n");

    // Create a new operation of the same type that the original one, that uses
    // the function arguments instead of dialect results of other ops
    // Helper - cast the quantum operation to the "SimpleCloneInterface" type, so
    // it is possible to invoke "simpleClone" on that operation
    {
        LLVM_DEBUG(llvm::dbgs() << "------------------------\n");
        OpBuilder::InsertionGuard g(*opBuilder);
        std::vector<Value> resultValues;
        for (Operation *quantumOp: quantumOps) {
            LLVM_DEBUG(llvm::dbgs() << "original op: " << *quantumOp << "\n");
            auto castedOldOp = dyn_cast<qmem::iface::SimpleCloneInterface>(quantumOp);
            assert(castedOldOp); // We expect that the cast will succeed

            // New operations (including the clone) should be attached to the block of the new function
            opBuilder->setInsertionPointToStart(newBlock);
            auto clonedOp = castedOldOp.simpleClone(*opBuilder, newFunc.getLoc());
            clonedOp->setOperands(newBlock->getArguments());

            // Populate the results from the current operation, matching the indexes already computed
            for (uint32_t resultIndex: typesAndValues.opResultsIndexes[quantumOp]) {
                resultValues.push_back(clonedOp->getResult(resultIndex));
            }
            LLVM_DEBUG(llvm::dbgs() << "cloned op: " << *clonedOp << "\n");
        }
        LLVM_DEBUG(llvm::dbgs() << "------------------------\n");

        // Create the "return" for the new operation
        auto returnOp = opBuilder->create<func::ReturnOp>(newFunc.getLoc(), resultValues);
        LLVM_DEBUG(llvm::dbgs() << "New Function: " << newFunc << "\n");
        LLVM_DEBUG(llvm::dbgs() << "************************\n");
    }
    return newFunc;
}

// Identifier to make the names of the new function definitions unique
static int identifier = 0;

void QMemSimpleFunctionizePass::runOnOperation() {
    ModuleOp module = dyn_cast<ModuleOp>(getOperation());
    assert(module); // We expect the cast to succeed

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
    Location topLocation = module.getBodyRegion().front().getOperations().front().getLoc();

    for (Operation *quantumOp : quantumOps) {
        // std::format was introduced as part of C++20 standard. This is included since GCC 13,
        // which is NOT part of the standard GCC package of ubuntu 22 (bundles GCC 11)
        // To avoid depending on headers included on a compiler that has to be manually installed,
        // or an extra library (like LLVM's libc++), we will default to good'ol 70's C way of
        // formatting a string
        // #include <format>
        //std::string funcName = std::format("__qoala_wrapper{}", identifier++);
        // Format the name of the new function
        auto nameFormat = "__qoala_wrapper%d";
        int length = std::snprintf(nullptr, 0, nameFormat, identifier);
        std::vector<char> funcName(length + 1);
        std::sprintf(funcName.data(), nameFormat, identifier++);

        // We create a new function that will contain the single operation in its body (+ a return statement)
        SetVector<Operation *> operations;
        operations.insert(quantumOp);
        func::FuncOp newFunc = createNewFunctionWithOperations(
                &opBuilder, std::string(funcName.data()), topLocation, operations
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
        LLVM_DEBUG(llvm::dbgs() << "Replacing: " << *quantumOp << "\n");
        LLVM_DEBUG(llvm::dbgs() << "With: " << callOp << "\n");
        quantumOp->replaceAllUsesWith(callOp);
        quantumOp->erase();
    }
}

std::unique_ptr<Pass> mlir::createQMemSimpleFunctionize() {
    return std::make_unique<QMemSimpleFunctionizePass>();
}
