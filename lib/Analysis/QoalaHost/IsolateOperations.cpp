#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Dialect/QMem/QMem.h"

#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "isolate-operations"

using namespace mlir;
using namespace qoala::dialects;

namespace qoala::analysis::isolate {
    static Operation *getNextOperation(Operation *op) {
        Block::iterator it(op);
        ++it;
        if (it != op->getBlock()->end()) {
            return &*it;
        }
        return nullptr;
    }

    static void isolateOp(Operation *opToIsolate, ConversionPatternRewriter &rewriter) {
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

            // Insert a new NopTOp at the end of the original block
            rewriter.setInsertionPoint(originalBlock, originalBlock->end());
            rewriter.create<qoalahost::NopTOp>(opToIsolate->getLoc());

            opToIsolateInNewBlock = &middleBlock->front();
            assert(opToIsolate == opToIsolateInNewBlock);
        }
        // Get the next operation to the opToIsolate
        Operation *nextToOpToIsolate = getNextOperation(opToIsolateInNewBlock);
        LLVM_DEBUG(llvm::dbgs() << "Next op to isolate: " << *nextToOpToIsolate << "\n");

        // Split the new block up to the next op to isolate.
        // In this way "nextToOpToIsolate" will be the first op of a new block, and
        // opToIsolate will be effectively isolated in a block (middleBlock)
        middleBlock->splitBlock(nextToOpToIsolate);
    }

    void isolateOpsInNewBlocks(Operation *funcOp, ConversionPatternRewriter &rewriter) {
        LLVM_DEBUG(llvm::dbgs() << "operation " << *funcOp << "\n");
        auto mainFunction = dyn_cast<qoalahost::MainFuncOp>(funcOp);
        assert (mainFunction && "Trying to isolate the operations on an operation that is not a qoalahost.main_func");
        SmallVector<Operation *> opsToIsolate;

        // To correctly isolate the op, we will simply "mark them", since modifying
        // the structure of the blocks will invalidate the internal iterators
        // of the walk functions.
        mainFunction.walk([&](qmem::RecvIntsOp op) {
            opsToIsolate.push_back(op.getOperation());
        });

        mainFunction.walk([&](qmem::RecvFloatsOp op) {
            opsToIsolate.push_back(op.getOperation());
        });

        mainFunction.walk([&](func::CallOp op) {
            // We will only isolate the "func.call" operations that have the
            // "functionaized" attribute, since they were created by the functionization
            // algorithm, hence, they will be mapped to "qoalahost.call" operation, calling
            // either a "netqasm.local_routine" or a "netqasm.request_routine" operations.
            if (Attribute attr = op->getAttr("functionized")) {
                if (const auto boolAttr = dyn_cast<BoolAttr>(attr); boolAttr.getValue()) {
                    opsToIsolate.push_back(op.getOperation());
                }
            }
        });

        // Once we marked all the ops, we isolate them
        for (Operation *opToIsolate : opsToIsolate) {
            isolateOp(opToIsolate, rewriter);
        }
    }
}