#ifndef QOALA_MLIR_CONVERSION_H
#define QOALA_MLIR_CONVERSION_H

#include "mlir/IR/BuiltinOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "Analysis/Helpers/SimpleCloneInterface.h"

#include <set>

namespace qoala::analysis {
    namespace functionize {
        struct ArgTypesAndReturns {
            std::vector<mlir::Type> argTypes;
            std::vector<mlir::Type> resultTypes;
            mlir::DenseMap<mlir::Operation *, std::set<uint32_t>> opResultsIndexes;
        };

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
        mlir::func::FuncOp createNewFunctionWithOperations(
                mlir::OpBuilder *opBuilder, mlir::StringRef funcName,
                mlir::Location loc, llvm::SetVector<mlir::Operation *> &quantumOps);

        /**
         * Analyzes the given set of operations to determine the "arguments" and "results" of the given set.
         * To this end the given operations will be treated as a "closure", meaning that all the data coming
         * from outside the set of operations will be considered an "argument". Similarly, any data that
         * is used outside the given set, will be considered as a result
         * @param operations The ser of operations to analyze.
         * @return An `ArgTypesAndReturn` struct, containing the argument types, result types, and a map
         *         between an operation (within the given set) that produces a result, and the set of indexes
         *         of that operation's result that are considered a result of the given operations set.
         */
        ArgTypesAndReturns computeArgTypesAndReturns(llvm::SetVector<mlir::Operation *> &operations);

        /**
         * "Functionizes" the operations on the given module. For each one of the operations
         * on the module, the given callback will be executed to determine if the involved
         * operation can be functionized or not.
         * @param module The module to analyze
         * @param opCanBeFunctionized A function that takes an operation, and determines if it
         *                            can be functionized or not.
         */
        void functionizeModule(mlir::ModuleOp &module, bool (*opCanBeFunctionized)(mlir::Operation *));
    }

    namespace flattening {
        void flattenFloatInstances(mlir::ModuleOp &module);
    }
}

#endif //QOALA_MLIR_CONVERSION_H
