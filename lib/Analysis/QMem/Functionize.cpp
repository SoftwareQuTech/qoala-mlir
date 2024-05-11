#include "Analysis/QMem/Conversion.h"
#include "Dialect/QMem/EntangleTrait.h"
#include "llvm/Support/Debug.h"

/* MLIR magic: using the LLVM_DEBUG macro + this define, allows to selectively enable the debug output
 * To enable the prints, simply invoke `opt` with `--debug-only=functionize-internal`, or any other string defined
 * as the "DEBUG_TYPE" macro */
#define DEBUG_TYPE "functionize-internal"

using namespace mlir;
using namespace qoala::analysis;

namespace qoala::analysis::functionize {
    // Identifier to make the names of the new function definitions unique
    static int identifier = 0;
    static inline std::string getNewFunctionName() {
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

        return {funcName.data()};
    }

    void computeArgTypesAndReturns(FunctionizeData &data, SetVector<Operation *> &quantumOps) {
        // We need to compute a sort of "closure" in the quantumOps set.
        // Create a new function at the top level, using the types of the "orphan" usages of the given ops:
        // If an operation that appears on the "uses" is not in the quantumOps set, then it's an
        // "outsider", hence it's an argument of the quantumOps set.
        // If an operation that appears on the "operands" is not in the quantumOps set, then it's an
        // "outsider", hence it's a result of the quantumOps set.
        std::vector<Type> argTypes;
        std::vector<Type> resultTypes;
        std::vector<Value> argumentValues;
        std::vector<Value> resultValues;

        for (Operation *quantumOp : quantumOps) {
            auto opOperands = quantumOp->getOperands();
            for (Value opOperand: opOperands) {
                if (!quantumOps.contains(opOperand.getDefiningOp())) {
                    // We discovered an argument of the group
                    argumentValues.push_back(opOperand);
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
                    resultValues.push_back(opResult);
                    resultTypes.push_back(opResult.getType());
                    returnIndexesForOp.insert(resultIndex);
                } else {
                    // If any of the result usages is not in the quantumOps (and that index is not on
                    // the already marked for return), then it needs to be returned
                    for (auto &usage: opResult.getUses()) {
                        if (!quantumOps.contains(usage.getOwner()) && !returnIndexesForOp.contains(resultIndex)) {
                            resultValues.push_back(opResult);
                            resultTypes.push_back(opResult.getType());
                            returnIndexesForOp.insert(resultIndex);
                        }
                    }
                }
            }
        }
        data.argTypes = argTypes;
        data.resultTypes = resultTypes;
        data.argumentValues.insert(argumentValues.begin(), argumentValues.end());
        data.resultValues.insert(resultValues.begin(), resultValues.end());
    }

    static inline void mapOriginalResultsInClonedOp(ResultRange originalResults, Operation *clonedOp,
                                                    SetVector<Value> &results,
                                                    DenseMap<OpResult, OpResult> &externalResultsMap,
                                                    DenseMap<Value, Value> &internalResultMap) {
        for (OpResult originalResult : originalResults) {
            OpResult correspondingNewResult = clonedOp->getResult(originalResult.getResultNumber());
            if (results.contains(originalResult)) {
                /* External usage of the result: Map the result of the old op to the corresponding results of the cloned op */
                externalResultsMap.insert(std::pair{originalResult, correspondingNewResult});
            }
            /* The result can _ALSO_ be used within the same closure of ops: map the old and new value  */
            internalResultMap.insert(std::pair{originalResult, correspondingNewResult});
        }
    }

    static inline void mapOriginalOperandsInClonedOp(MutableArrayRef<OpOperand> originalOpOperands,
                                                     Operation *clonedOp, SetVector<Value> &arguments,
                                                     std::vector<Value> &clonedOpOperands, func::FuncOp &newFunc,
                                                     DenseMap<Value, Value> internalResultMap) {
        for (OpOperand &originalOperand : originalOpOperands) {
            unsigned int originalOpIdx = originalOperand.getOperandNumber();
            Value newArgument = clonedOp->getOperand(originalOpIdx);
            if (arguments.contains(originalOperand.get())) {
                /* Use of an external operand (argument): use the argument of the new function as operand */
                clonedOpOperands.push_back(newFunc.getArgument(originalOpIdx));
            } else {
                /* Use of an internal operand: use the operand as mapped by the internalResultsMap */
                Value origOpValue = originalOperand.get();
                clonedOpOperands.push_back(internalResultMap[origOpValue]);
            }
        }
    }

    static ValueRange consolidateResults(DenseMap<Value, Value> &internalResultMap) {
        std::vector<Value> functionResults;
        for (auto resultMapPair : internalResultMap) {
            functionResults.push_back(resultMapPair.getSecond());
        }
        return {functionResults};
    }

    void createNewFunctionWithOperations(FunctionizeData &data,
                                         OpBuilder *opBuilder, StringRef funcName,
                                         Location loc, SetVector<Operation *> &quantumOpsGroup) {
        std::vector<Operation *> clonedOperations;
        DenseMap<OpResult, OpResult> externalResultsMap;
        DenseMap<Value, Value> internalResultMap;

        computeArgTypesAndReturns(data, quantumOpsGroup);

        // Create the new function, and attach a block to it
        FunctionType funType = opBuilder->getFunctionType(data.argTypes, data.resultTypes);
        auto newFunc = opBuilder->create<func::FuncOp>(loc, funcName, funType);
        Block *newBlock = newFunc.addEntryBlock();
        bool hasEntanglementOp = false;

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
            for (Operation *originalQuantumOp : quantumOpsGroup) {
                LLVM_DEBUG(llvm::dbgs() << "original op: " << *originalQuantumOp << "\n");

                auto castedOldOp = dyn_cast<helpers::SimpleCloneInterface>(originalQuantumOp);
                assert(castedOldOp); // We expect that the cast will succeed

                // New operations (including the clone) should be attached to the block of the new function
                opBuilder->setInsertionPointToStart(newBlock);
                Operation *clonedOp = castedOldOp.simpleClone(*opBuilder, newFunc.getLoc());
                clonedOperations.push_back(clonedOp);
                std::vector<Value> clonedOpOperands;

                /* Process the operands of the old op */
                mapOriginalOperandsInClonedOp(originalQuantumOp->getOpOperands(), clonedOp,
                                              data.argumentValues, clonedOpOperands,
                                              newFunc, internalResultMap);

                /* Process the results of the old op */
                mapOriginalResultsInClonedOp(originalQuantumOp->getResults(), clonedOp,
                                             data.resultValues, externalResultsMap, internalResultMap);

                // Then, we need to set the operands of the cloned operation
                clonedOp->setOperands(clonedOpOperands);

                // If the current operation has the "Entangle" trait, then the function wrapper needs
                // to have the "entangle" attribute. This will be used later when lowering to NetQASM
                if (originalQuantumOp->hasTrait<mlir::OpTrait::Entangle>()) {
                    hasEntanglementOp = true;
                }

                LLVM_DEBUG(llvm::dbgs() << "cloned op: " << *clonedOp << "\n");
            }
            if (hasEntanglementOp) {
                // We "mark" the function declaration, so when lowering the func operation the pass can
                // know that this particular function needs to be lowered to netqasm.request_routine
                Attribute entangle = opBuilder->getStringAttr("true");
                newFunc->setAttr("entangle", entangle);
            }
            LLVM_DEBUG(llvm::dbgs() << "------------------------\n");

            // Create the "return" for the new operation
            std::vector<Value> functionResults;
            for (auto resultMapPair : externalResultsMap) {
                Value originalResult = resultMapPair.getFirst();
                Value functionResult = resultMapPair.getSecond();
                // Hack: the position of this argument will be exactly the current size of the functionResults vector
                data.replacementMap.insert(std::pair{originalResult, functionResults.size()});
                functionResults.push_back(functionResult);
            }
            auto returnOp = opBuilder->create<func::ReturnOp>(newFunc.getLoc(),
                                                              functionResults);
            LLVM_DEBUG(llvm::dbgs() << "New Function: " << newFunc << "\n");
            LLVM_DEBUG(llvm::dbgs() << "Has entanglement: " << hasEntanglementOp << "\n");
            LLVM_DEBUG(llvm::dbgs() << "Return op: " << returnOp << "\n");
            LLVM_DEBUG(llvm::dbgs() << "************************\n");
        }
        data.newFunction = newFunc;
    }

    void functionizeModule(ModuleOp &module, BucketsTy (*classifyOperations)(ModuleOp *)) {
        std::map<std::vector<Operation *>, func::FuncOp> mappedFunctions;
        BucketsTy functionGroups = classifyOperations(&module);

        // We start building our new functions at the start of the body of the module
        OpBuilder opBuilder(module.getBodyRegion());
        Location topLocation = module.getBodyRegion().front().getOperations().front().getLoc();

        for (std::vector<Operation *> opGroup : functionGroups) {
            FunctionizeData data;
            std::string newFuncName = getNewFunctionName();

            // We create a new function that will contain the single operation in its body (+ a return statement)
            SetVector<Operation *> operations;
            operations.insert(opGroup.begin(), opGroup.end());
            createNewFunctionWithOperations(
                    data, &opBuilder, newFuncName, topLocation, operations
            );
            auto insertedSymbol = module.lookupSymbol<func::FuncOp>(data.newFunction.getNameAttr());
            mappedFunctions.insert(std::pair{opGroup, insertedSymbol});

            // Create a "call" operation in place of the original operations
            OpBuilder::InsertionGuard g(opBuilder);
            opBuilder.setInsertionPointAfter(opGroup[0]);
            auto callOp = opBuilder.create<func::CallOp>(
                    opGroup[0]->getLoc(), mappedFunctions[opGroup],
                    /*args=*/data.argumentValues.getArrayRef()
            );

            // Finally, replace the usages of the old operation with the result of the "call"
            // and delete the old operation
            for (Operation *quantumOp : opGroup) {
                LLVM_DEBUG(llvm::dbgs() << "Replacing: " << *quantumOp << "\n");
            }
            LLVM_DEBUG(llvm::dbgs() << "With: " << callOp << "\n");
            // Replace the uses of the functionized operations with the results returned by the call operation
            auto callResults = callOp->getResults();
            for (auto pair : data.replacementMap) {
                Value oldValue = pair.getFirst();
                oldValue.replaceAllUsesWith(callOp.getResult(pair.getSecond()));
            }
            // Finally, delete all the operations of the group
            for (Operation *op : opGroup) {
                op->erase();
            }
        }
    }
}