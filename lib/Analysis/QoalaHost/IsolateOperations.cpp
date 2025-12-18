#include "Analysis/QoalaHost/Isolate.h"
#include "Dialect/QoalaHost/QoalaHost.h"

#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "isolate-operations"

using namespace mlir;
using namespace qoala::dialects;

namespace qoala::analysis::isolate {
    Operation *getNextOperation(Operation *op) {
        Block::iterator it(op);
        ++it;
        if (it != op->getBlock()->end()) {
            return &*it;
        }
        return nullptr;
    }

    void isolateOp(Operation *opToIsolate, ConversionPatternRewriter &rewriter) {
        LLVM_DEBUG(llvm::dbgs() << "Isolating operation " << *opToIsolate << "\n");
        Block *originalBlock = opToIsolate->getBlock();
        Operation *opToIsolateInNewBlock;
        Block *middleBlock;

        if (&originalBlock->front() == opToIsolate) {
            // Special case - The operation to isolate is the first of the block
            LLVM_DEBUG(llvm::dbgs() << "op first of block" << *opToIsolate << "\n");
            // We don't need to insert a NopTOp, but simply split the original block at
            // the operation immediately next to opToIsolate;
            opToIsolateInNewBlock = opToIsolate;
            middleBlock = originalBlock;
        } else {
            // We split the block up to the given op (will be the first op of the returned block)
            middleBlock = originalBlock->splitBlock(opToIsolate);

            // Insert a new NopTOp at the end of the original block if needed
            if (!originalBlock->mightHaveTerminator()) {
                rewriter.setInsertionPoint(originalBlock, originalBlock->end());
                rewriter.create<qoalahost::NopTOp>(opToIsolate->getLoc());
            }

            opToIsolateInNewBlock = &middleBlock->front();
            assert(opToIsolate == opToIsolateInNewBlock);
        }
        // Get the next operation to the opToIsolate
        Operation *nextToOpToIsolate = getNextOperation(opToIsolateInNewBlock);
        LLVM_DEBUG(llvm::dbgs() << "Next op to isolate: " << *nextToOpToIsolate << "\n");

        // Split the new block up to the next op to isolate.
        // In this way "nextToOpToIsolate" will be the first op of a new block, and
        // opToIsolate will be effectively isolated in a block (middleBlock)
        middleBlock->splitBlock(nextToOpToIsolate); /*=bottomBlock*/

        // Insert a new NopTOp at the end of the middle block if needed
        if (!middleBlock->mightHaveTerminator()) {
            rewriter.setInsertionPoint(middleBlock, middleBlock->end());
            rewriter.create<qoalahost::NopTOp>(nextToOpToIsolate->getLoc());
        }

        // We do not need to add a terminator op on the bottomBlock, since all the
        // lower operations that were in the original block (including the original
        // terminator) will end up in bottomBlock
    }

    void createNewEmptyFirstBlock(ConversionPatternRewriter &rewriter, qoalahost::MainFuncOp &mainFunc) {
        Block &firstBlock = mainFunc.front();
        Operation &firstOperation = firstBlock.front();
        // We split the first block at the first operation. This method returns a new Block pointer,
        // which contains all (and including) the operation at which we splat.
        // The original block ("firstBlock") will contain everything that was *before* the split point.
        // Hence, after this split, teh first block should be empty.
        firstBlock.splitBlock(&firstOperation);
        // We also have to insert a terminator operation in the empty block, so the IR stays valid
        rewriter.setInsertionPoint(&firstBlock, firstBlock.begin());
        rewriter.create<qoalahost::NopTOp>(firstOperation.getLoc());
    }

    void removeFirstBlockFromMainFuncIfEmpty(ModuleOp &module) {
        const auto mainFuncs = module.getOps<qoalahost::MainFuncOp>();
        assert(!mainFuncs.empty() && "No main function? This is embarrassing");
        auto mainFunc = *mainFuncs.begin();
        if (Block &firstBlock = mainFunc.front(); firstBlock.getOperations().size() == 1) {
            assert(isa<qoalahost::NopTOp>(firstBlock.front()) && "Single operation of an empty"
                                                                 "block is not a NopTOp.");
            firstBlock.erase();
        }
    }
} // namespace qoala::analysis::isolate
