#include <llvm/ADT/SmallSet.h>
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
        llvm::SetVector<Value> argumentValues;
        llvm::SetVector<Value> resultValues;
        llvm::DenseMap <Value, unsigned int> externalArgsIdxMap;

        for (Operation *quantumOp : quantumOps) {
            OperandRange opOperands = quantumOp->getOperands();
            for (Value opOperand: opOperands) {
                // We check if the operation defining the current value is not within the same quantumOpsGroup
                // and if the current value was not discovered as an external value already.
                if (!quantumOps.contains(opOperand.getDefiningOp()) && !argumentValues.contains(opOperand)) {
                    // We discovered an external argument of the group
                    // Map the discovered value to the function argument index that will correspond to the same value
                    // Luckily we know that its position will be exactly the current size of the argumentValues vector
                    externalArgsIdxMap.insert(std::pair{opOperand, argumentValues.size()});
                    // Add the value and its type to the discovered sets
                    argumentValues.insert(opOperand);
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
                    resultValues.insert(opResult);
                    resultTypes.push_back(opResult.getType());
                    returnIndexesForOp.insert(resultIndex);
                } else {
                    // If any of the result usages is not in the quantumOps (and that index is not on
                    // the already marked for return), then it needs to be returned
                    for (OpOperand &usage: opResult.getUses()) {
                        if (!quantumOps.contains(usage.getOwner()) && !returnIndexesForOp.contains(resultIndex)) {
                            // We discovered an external result of the group
                            resultValues.insert(opResult);
                            resultTypes.push_back(opResult.getType());
                            returnIndexesForOp.insert(resultIndex);
                        }
                    }
                }
            }
        }
        data.argTypes = argTypes;
        data.resultTypes = resultTypes;
        data.externalArgValsIdxMap = externalArgsIdxMap;
        data.externalArgsVals.insert(argumentValues.begin(), argumentValues.end());
        data.externalResVals.insert(resultValues.begin(), resultValues.end());
    }

    static inline void mapOriginalResultsInClonedOp(ResultRange originalResults, Operation *clonedOp,
                                                    SetVector<Value> &externalResults,
                                                    DenseMap<OpResult, OpResult> &externalResultsMap,
                                                    DenseMap<Value, Value> &internalResultMap) {
        for (OpResult originalResult : originalResults) {
            OpResult correspondingNewResult = clonedOp->getResult(originalResult.getResultNumber());
            if (externalResults.contains(originalResult)) {
                /* External usage of the result: Map the result of the old op to the corresponding results of the cloned op */
                externalResultsMap.insert(std::pair{originalResult, correspondingNewResult});
            }
            /* The result can _ALSO_ be used within the same closure of ops: map the old and new value  */
            internalResultMap.insert(std::pair{originalResult, correspondingNewResult});
        }
    }

    static inline void mapOriginalOperandsInClonedOp(OperandRange originalOpOperands,
                                                     DenseMap<Value, unsigned int> &externalArgValsIdxMap,
                                                     SetVector<Value> &externalArguments,
                                                     std::vector<Value> &clonedOpOperands, func::FuncOp &newFunc,
                                                     DenseMap<Value, Value> internalResultMap) {
        for (Value operandValue : originalOpOperands) {
            if (externalArguments.contains(operandValue)) {
                /* Use of an external operand (argument): use the argument of the new function as operand */
                clonedOpOperands.push_back(newFunc.getArgument(externalArgValsIdxMap[operandValue]));
            } else {
                /* Use of an internal operand: use the operand as mapped by the internalResultsMap */
                clonedOpOperands.push_back(internalResultMap[operandValue]);
            }
        }
    }

    void createNewFunctionWithOperations(FunctionizeData &data,
                                         OpBuilder &opBuilder, StringRef funcName,
                                         Location loc, SetVector<Operation *> &quantumOpsGroup) {
        std::vector<Operation *> clonedOperations;
        DenseMap<OpResult, OpResult> externalResultsMap;
        DenseMap<Value, Value> internalResultMap;

        computeArgTypesAndReturns(data, quantumOpsGroup);

        // Create the new function, and attach a block to it
        FunctionType funType = opBuilder.getFunctionType(data.argTypes, data.resultTypes);
        auto newFunc = opBuilder.create<func::FuncOp>(loc, funcName, funType);
        Block *newBlock = newFunc.addEntryBlock();
        bool functionHasEPRSOp = false;

        LLVM_DEBUG(llvm::dbgs() << "------------------------\n");
        LLVM_DEBUG(llvm::dbgs() << "Discovered func type: " << funType << "\n");

        // Create a new operation of the same type that the original one, that uses
        // the function arguments instead of dialect results of other ops
        // Helper - cast the quantum operation to the "SimpleCloneInterface" type, so
        // it is possible to invoke "simpleClone" on that operation
        {
            LLVM_DEBUG(llvm::dbgs() << "------------------------\n");
            LLVM_DEBUG(llvm::dbgs() << "Cloning ops:\n");
            OpBuilder::InsertionGuard g(opBuilder);
            // New operations (including the clone) should be attached to the block of the new function
            opBuilder.setInsertionPointToStart(newBlock);

            for (Operation *originalQuantumOp : quantumOpsGroup) {
                LLVM_DEBUG(llvm::dbgs() << " - original op: " << *originalQuantumOp << "\n");

                auto castedOldOp = dyn_cast<helpers::SimpleCloneInterface>(originalQuantumOp);
                assert(castedOldOp); // We expect that the cast will succeed
                Operation *clonedOp = castedOldOp.simpleClone(opBuilder, newFunc.getLoc());

                /* Process the operands of the old op */
                std::vector<Value> clonedOpOperands;
                mapOriginalOperandsInClonedOp(originalQuantumOp->getOperands(),
                                              data.externalArgValsIdxMap, data.externalArgsVals,
                                              clonedOpOperands, newFunc, internalResultMap);

                /* Process the results of the old op */
                mapOriginalResultsInClonedOp(originalQuantumOp->getResults(), clonedOp,
                                             data.externalResVals, externalResultsMap, internalResultMap);

                // Then, we need to set the operands of the cloned operation
                clonedOp->setOperands(clonedOpOperands);

                // If the current operation has the "Entangle" trait, then the function wrapper needs
                // to have the "entangle" attribute. This will be used later when lowering to NetQASM
                if (originalQuantumOp->hasTrait<mlir::OpTrait::Entangle>()) {
                    functionHasEPRSOp = true;
                }

                LLVM_DEBUG(llvm::dbgs() << " * cloned op: " << *clonedOp << "\n");
            }
            if (functionHasEPRSOp) {
                // We "mark" the function declaration, so when lowering the func operation the pass can
                // know that this particular function needs to be lowered to netqasm.request_routine
                Attribute entangle = opBuilder.getStringAttr("true");
                newFunc->setAttr("entangle", entangle);
            }
            LLVM_DEBUG(llvm::dbgs() << "------------------------\n");

            // Consolidate the external results of the function, mapping the original result with the index of the
            // operand in the return instruction. This index will map 1-to-1 to the results of the "call" op.
            // At this time, we also consolidate the external function results (values) that will be used as
            // operands of the return operation.
            std::vector<Value> functionResults;
            for (auto resultMapPair : externalResultsMap) {
                Value originalResult = resultMapPair.getFirst();
                Value functionResult = resultMapPair.getSecond();
                // Hack: the position of this argument will be exactly the current size of the functionResults vector
                data.replacementMap.insert(std::pair{originalResult, functionResults.size()});
                functionResults.push_back(functionResult);
            }
            // Create the "return" for the new operation
            auto returnOp = opBuilder.create<func::ReturnOp>(newFunc.getLoc(),
                                                              functionResults);
            LLVM_DEBUG(llvm::dbgs() << "New Function:\n" << newFunc << "\n");
            LLVM_DEBUG(llvm::dbgs() << "Has entanglement: " << (functionHasEPRSOp ? "Yes" : "No") << "\n");
            LLVM_DEBUG(llvm::dbgs() << "Return op:\n - " << returnOp << "\n");
            LLVM_DEBUG(llvm::dbgs() << "------------------------\n");
        }
        data.newFunction = newFunc;
    }

    void functionizeModule(ModuleOp &module, ClassifierFnTy classifyOperations) {
        unsigned int groupNum = 0;

        auto mainFunctions = module.getOps<dialects::qmem::FuncOp>();
        // We expect at least one qmem::FuncOp operation in the module
        assert(!mainFunctions.empty());
        // We assume that the main function to analyze is just the first one (lexicographically) in the module
        dialects::qmem::FuncOp mainFunction = *mainFunctions.begin();
        std::vector<QuantumOpsGroupTy> functionGroups = classifyOperations(mainFunction);

        // We start building our new functions at the start of the body of the module
        OpBuilder opBuilder(module.getBodyRegion());
        Location topLocation = module.getBodyRegion().front().getOperations().front().getLoc();

        for (QuantumOpsGroupTy quantumOpsGroup : functionGroups) {
            LLVM_DEBUG(llvm::dbgs() << "------------------------\n");
            LLVM_DEBUG(llvm::dbgs() << "Process group #" << groupNum << "\n");

            // The container structure for the funcitonize data
            FunctionizeData data;
            std::string newFuncName = getNewFunctionName();

            // We create a new function that will contain all the operations in its body (+ a return statement)
            SetVector<Operation *> operations;
            operations.insert(quantumOpsGroup.begin(), quantumOpsGroup.end());
            createNewFunctionWithOperations(
                    data, opBuilder, newFuncName, topLocation, operations
            );
            // Search for the symbol (func.func object) that we just inserted
            auto insertedSymbol = module.lookupSymbol<func::FuncOp>(data.newFunction.getNameAttr());

            // Create a "call" operation in place of the original operations
            OpBuilder::InsertionGuard g(opBuilder);
            // IMPORTANT: We insert the call operation _AFTER_ the last operation of the group.
            // By placing it there, we are sure that all data dependencies are fulfilled.
            opBuilder.setInsertionPointAfter(*quantumOpsGroup.rbegin());
            auto callOp = opBuilder.create<func::CallOp>(
                    quantumOpsGroup[0]->getLoc(), insertedSymbol,
                    /*args=*/data.externalArgsVals.getArrayRef()
            );

            LLVM_DEBUG(llvm::dbgs() << "Replacing operations in group #" << groupNum++ << ":\n");
            for (Operation *quantumOp : quantumOpsGroup) {
                LLVM_DEBUG(llvm::dbgs() << " -> " << *quantumOp << "\n");
            }
            LLVM_DEBUG(llvm::dbgs() << "With:\n * " << callOp << "\n");

            // Replace the uses of the functionized operations with the results returned by the call operation
            for (auto pair : data.replacementMap) {
                Value oldValue = pair.getFirst();
                oldValue.replaceAllUsesWith(callOp.getResult(pair.getSecond()));
            }

            // Finally, delete all the operations of the group IN REVERSE ORDER to avoid leaving
            // operations using deleted values in the middle of the deletion (MLIR will complain about that)
            std::reverse(quantumOpsGroup.begin(), quantumOpsGroup.end());
            for (Operation *op : quantumOpsGroup) {
                op->erase();
            }
        }
    }
}