#ifndef HELPERS_H
#define HELPERS_H

#include "llvm/Support/Casting.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/IR/BuiltinOps.h"

namespace qoala::analysis {
    namespace isolate {
        mlir::Operation *getNextOperation(mlir::Operation *op);
        /**
         * Isolate the given operation in its own block.
         * @param opToIsolate Operation to isolate in its own block.
         * @param rewriter The convertion patter rewriter to create new block
         * terminators as needed.
         */
        void isolateOp(mlir::Operation *opToIsolate, mlir::ConversionPatternRewriter &rewriter);

        using OpCheck = bool (*)(mlir::Operation *);

        /**
         * Isolate all the nested operations of a given type. The search will start from
         * a given base operation and isolate all the found operations into a single
         * block. Since creating new isolated blocks might create new blocks with no
         * terminator operations, this method will also create new terminator operations
         * as needed. This function also allows passing an extra check function to
         * exclude the isolation of certain nested operations despite they match the
         * types.
         * @tparam FuncOpTy Type of the base function to start exploring for nested
         * operations. This type should usually be "qoalahost::MainFuncOp", but can
         * easily be other type with a nested region.
         * @tparam OpTy Types of the operations to isolate.
         * @param baseOp The base operation to explore for functions to isolate.
         * @param rewriter A ConversionPatternRewriter object to create new block
         * terminators as needed
         * @param extraCheck An optional extra check function. If this given check
         * returns false, the involved operation will not be marked to isolate, even if
         * it is of the good type.
         */
        template<typename FuncOpTy, typename... OpTy>
        void isolateOpsInNewBlocks(FuncOpTy &baseOp, mlir::ConversionPatternRewriter &rewriter,
                                   const std::optional<OpCheck> extraCheck = std::nullopt) {
            auto mainFunction = llvm::dyn_cast<FuncOpTy>(&baseOp);
            assert(mainFunction && "Trying to isolate the operations on an operation "
                                   "that is not a qoalahost.main_func");
            mlir::SmallVector<mlir::Operation *> opsToIsolate;

            // To correctly isolate the op, we will simply "mark them", since modifying
            // the structure of the blocks will invalidate the internal iterators
            // of the walk functions.
            mainFunction->walk([&](mlir::Operation *op) {
                if (llvm::isa<OpTy...>(op)) {
                    if (extraCheck.has_value() && !extraCheck.value()(op)) {
                        return;
                    }
                    opsToIsolate.push_back(op);
                }
            });

            // Once we marked all the ops, we isolate them
            for (mlir::Operation *opToIsolate: opsToIsolate) {
                isolateOp(opToIsolate, rewriter);
            }
        }
    } // namespace isolate

    namespace precedences {
        /**
         * Track precedences (data dependencies, predecessors, communication/entanglement ordering).
         * Make the precedences information available by inserting a "blk_meta"
         * operation At the beginning of each block.
         * @param moduleOp module to walk for tracking and adding precedences.
         */
        mlir::LogicalResult addPrecedences(mlir::ModuleOp &moduleOp);
    } // namespace precedences
} // namespace qoala::analysis

#endif // HELPERS_H
