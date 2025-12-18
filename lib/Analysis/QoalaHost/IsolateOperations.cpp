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
        // Check if any of the declared remotes is used; If it is, we know we have to create an empty
        // block for the socket IDs placeholders
        auto module = mainFunc->getParentOfType<ModuleOp>();
        const auto remoteDeclarations = module.getOps<qremote::RemoteOp>();
        const bool anyRemoteIsUsed =
                std::any_of(remoteDeclarations.begin(), remoteDeclarations.end(), [&](qremote::RemoteOp op) {
                    if (!op.getSymbolUses(mainFunc)->empty()) {
                        return true;
                    }
                    return false;
                });
        if (!anyRemoteIsUsed) {
            return;
        }
        // At this point we know we have to create an empty block
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
        Block &firstBlock = mainFunc.front();
        if (firstBlock.getOperations().size() == 1 && isa<qoalahost::NopTOp>(firstBlock.front())) {
            // This is a safety check. When translating the qmem:FuncOp into qoalahost::MainFuncOp we
            // create the empty block for the socket IDs *only* if the remote (symbol) is used somewhere
            // inside the MainFuncOp. If this is not the case, the block is not created.
            // This additional avoids ending up with a case where we need to delete an unnecessary block
            // that defines arguments of the main function. Deleting such a block would result on a failure,
            // due to the block still having "uses" (use of the arguments).
            for (auto blockArgument : firstBlock.getArguments()) {
                assert(!blockArgument.getUses().empty() && "Trying to delete a frist block whose arguments"
                                                           "has uses. This is a bug on the implementation.");
            }
            // We only delete the first block, if it has one operation, and it is a NopTOp
            firstBlock.erase();
        }
    }
} // namespace qoala::analysis::isolate
