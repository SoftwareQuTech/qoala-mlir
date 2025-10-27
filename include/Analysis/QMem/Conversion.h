#ifndef QOALA_MLIR_CONVERSION_H
#define QOALA_MLIR_CONVERSION_H

#include "Dialect/QMem/QMem.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinOps.h"

namespace qoala::analysis::functionize {
    struct FunctionizeData {
        /* Types for the arguments and results */
        std::vector<mlir::Type> argTypes;
        std::vector<mlir::Type> resultTypes;
        /* Values of the external arguments and external results discovered */
        mlir::SetVector<mlir::Value> externalArgsVals;
        mlir::SetVector<mlir::Value> externalResVals;
        /* Map between the external arguments and the corresponding argument index of the new function */
        mlir::DenseMap<mlir::Value, unsigned int> externalArgValsIdxMap;
        /* The new function created */
        mlir::func::FuncOp newFunction;
        /* Map between the results of the functionized group (original results) and the _index_
         * of the "call" operation, which corresponds to the _new_ result */
        mlir::DenseMap<mlir::Value, int> replacementMap;
    };

    using QuantumOpsGroupTy = std::vector<mlir::Operation *>;
    using ClassifierFnTy = std::vector<QuantumOpsGroupTy> (*)(dialects::qmem::FuncOp &, uint32_t);

    /**
     * Creates a "wrapper" function around the given list of operations.
     * WARNING: This function will not perform any type of Data nor Control Flow
     * analysis on the given operations, and it assumes that the data and control
     * flow of the given list is consistent.
     * WARNING: This function _assumes_ that `computeTypesAndReturns` was called with the
     * same `data` object as parameter, so some members of that struct are _assumed_ to be
     * set and/or initialized.
     * @param data A `FunctionizeData` struct that already contains the data about the args and
     *             return types, as well as the map of the operation indexes. Invoking this
     *             function will use this information to set the `newFunction` and `finalResultIndexMap`
     *             members.
     * @param opBuilder An `OpBuilder` object used to clone the given operations.
     * @param funcName The name of the function to be created.
     * @param loc The location to use as an attribute for the new operations created.
     * @param quantumOpsGroup An ordered `SetVector` object containing the set of operations to wrap, in the
     *                        order they need to be inserted in the new body.
     */
    void createNewFunctionWithOperations(FunctionizeData &data, mlir::OpBuilder &opBuilder, mlir::StringRef funcName,
                                         const mlir::Location &loc,
                                         llvm::SetVector<mlir::Operation *> &quantumOpsGroup);

    /**
     * Analyzes the given set of operations to determine the "arguments" and "results" of the given set.
     * To this end the given operations will be treated as a "closure", meaning that all the data coming
     * from outside the set of operations will be considered an "argument". Similarly, any data that
     * is used outside the given set, will be considered as a result
     * @param data A `FunctionizeData` struct, that will contain the argument types, result types,
     *             and a map between an operation (within the given set) that produces a result, and
     *             the set of indexes of that operation's result that are considered a result of the
     *             given operations set. After calling this function, any other member of this struct
     *             are not guaranteed to be set or initialized.
     * @param operations An _ordered_ set of of operations to analyze.
     */
    void computeArgTypesAndReturns(FunctionizeData &data, llvm::SetVector<mlir::Operation *> &operations);

    /**
     * "Functionizes" the operations on the given module. For each one of the operations
     * on the module, the given callback will be executed to determine if the involved
     * operation can be functionized or not.
     * @param module The module to analyze
     * @param operationsClassifier A function of type std::vector<QuantumOpsGroupTy> (*)(dialects::qmem::FuncOp &).
     *                             This function should iterate over all the operations of the given FuncOp and
     *                             classify them into a collection of QuantumOpsGroupTy. Each one of the operation
     *                             groups will be converted into a single function.
     * @param maxOpsPerGroup Maximum number of operations to leave inside a group.
     */
    void functionizeModule(mlir::ModuleOp &module, ClassifierFnTy operationsClassifier, uint32_t maxOpsPerGroup);

    /**
     * Classifier to create *simple* groups, i.e., each group will contain _a single quantum operation_.
     * This classifier is used with the simple functionization
     * @param mainFunction The function on which run the grouping
     * @param maxOpsPerGroup Maximum number of operations to leave inside a group. This argument is ignored.
     * @return A collection of quantum groups.
     */
    std::vector<QuantumOpsGroupTy> simpleOpClassifier(dialects::qmem::FuncOp &mainFunction, uint32_t maxOpsPerGroup);

    /**
     * Operation classifier that follows the rules of the compiler specifications. This method creates
     * operation groups based on the criteria specified in
     * https://gitlab.tudelft.nl/qoala/qoala-compiler-spec/-/blob/master/design/translations/MIR_to_LIR.md
     * @param mainFunction The function on which run the grouping
     * @param maxOpsPerGroup Maximum number of operations to leave inside a group.
     * @return A collection of quantum groups.
     */
    std::vector<QuantumOpsGroupTy> functionizeOpClassifier(dialects::qmem::FuncOp &mainFunction,
                                                           uint32_t maxOpsPerGroup);
} // namespace qoala::analysis::functionize

#endif // QOALA_MLIR_CONVERSION_H
