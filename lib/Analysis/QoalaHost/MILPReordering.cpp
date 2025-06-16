#include "Analysis/QoalaHost/Helpers.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "llvm/Support/Debug.h"
#include "mlir/IR/SymbolTable.h"
#include "Dialect/QoalaHost/Passes.h"

#define DEBUG_TYPE "qoalahost-reorder-blocks-pass-internal"

using namespace mlir;
using namespace qoala::dialects;
using namespace qoala::analysis;

namespace qoala::analysis::reordering {

    std::pair<mlir::LogicalResult, std::vector<std::unique_ptr<MILPBlock>>> buildMILPBlocks(mlir::ModuleOp moduleOp) {
        // Constructs MILPBlock representations from all blocks within `MainFuncOp`.
        //
        // Each `MILPBlock` captures the semantic role and metadata for a block in the
        // quantum control program, and is used for MILP-based reordering.
        //
        // ### Assumptions:
        // - There exists exactly one `qoalahost::MainFuncOp` in the module.
        // - Each block in the main function must begin with a `qoalahost::BlkMeta` operation.
        // - Each block can be categorized into one of three types: `CL`, `CC`, `QL`, or `QC`.
        // - For `QL` and `QC` blocks, the called region must end with a `netqasm::ReturnOp`.
        //
        // ### Block Types:
        // - **QC** (quantum communication): if the block calls a `RequestRoutine`.
        // - **QL** (quantum local): if the block calls a `LocalRoutine`.
        // - **CC** (classical communication): if the block contains a `Recv` operation.
        // - **CL** (classical local): all other blocks.
        //
        // ### Error Conditions:
        // - A block does not start with `BlkMeta` → emits error, returns `failure()`.
        // - A `CallOp` callee is invalid or lacks a return → emits error, returns `failure()`.
        //
        // ### Output:
        // - `std::pair<LogicalResult, DenseMap>` where:
        //   - `LogicalResult` is `failure()` if any block is malformed.
        //   - `DenseMap` maps MLIR blocks to corresponding `MILPBlock` models.
        std::vector<std::unique_ptr<MILPBlock>> milpBlocks;

        mlir::SmallVector<qoalahost::MainFuncOp, 1> mainFuncs;
        for (qoalahost::MainFuncOp func: moduleOp.getOps<qoalahost::MainFuncOp>()) {
            mainFuncs.push_back(func);
        }

        if (mainFuncs.empty()) {
            mlir::emitError(moduleOp.getLoc(), "No qoalahost::MainFuncOp found in module");
            std::vector<std::unique_ptr<MILPBlock>> emptyVec;
            return std::pair<mlir::LogicalResult, std::vector<std::unique_ptr<MILPBlock>>>(mlir::failure(),
                                                                                           std::move(emptyVec));
        }

        qoalahost::MainFuncOp mainFunc = mainFuncs.front();
        LLVM_DEBUG(llvm::dbgs() << "[MILP] Building MILPBlocks from MainFuncOp\n");

        for (mlir::Block &block: mainFunc.getBody()) {
            // For each block in the main function, we attempt to build a MILPBlock.
            //
            // All valid blocks are expected to begin with a BlkMeta operation that stores
            // the block ID and metadata (predecessors, dependencies, prev_comm, prev_ent).
            //
            // We classify the block based on the type of operation that immediately follows
            // the BlkMeta, and extract relevant sub-operations for analysis or modeling.

            mlir::Operation *firstOp = &block.front();
            if (!llvm::isa<qoalahost::BlkMeta>(firstOp)) {
                // Ensure that the first operation in the block is a BlkMeta.
                // This is a strict structural requirement for MILP analysis.
                // If it's missing, we emit an error and abort the pass.

                mlir::emitError(firstOp->getLoc(), "Expected BlkMeta as first operation in block");
                std::vector<std::unique_ptr<MILPBlock>> emptyVec;
                return std::pair<mlir::LogicalResult, std::vector<std::unique_ptr<MILPBlock>>>(mlir::failure(),
                                                                                               std::move(emptyVec));
            }

            qoalahost::BlkMeta blkMeta = llvm::cast<qoalahost::BlkMeta>(firstOp);

            std::string blockId = blkMeta.getBlockId().str();

            std::vector<std::string> preds;
            for (mlir::Attribute attr: blkMeta.getPredecessors()) {
                preds.push_back(attr.cast<mlir::StringAttr>().getValue().str());
            }

            std::vector<std::string> deps;
            for (mlir::Attribute attr: blkMeta.getDependencies()) {
                deps.push_back(attr.cast<mlir::StringAttr>().getValue().str());
            }

            std::string prevComm = blkMeta.getPrevComm().str();
            std::string prevEnt = blkMeta.getPrevEnt().str();

            BlockType blockType = BlockType::CL;

            mlir::Block::iterator it = std::next(block.begin()); // skip BlkMeta
            if (it == block.end()) {
                LLVM_DEBUG(llvm::dbgs() << "[MILP] Block " << blockId << " has only BlkMeta\n");
                continue;
            }

            mlir::Operation *op = &*it;
            int opIndex = 0;

            // --- Case 1: CallOp → QL or QC ---
            // This block begins with a CallOp, meaning it's a QL or QC block.
            // We'll follow the call to the corresponding routine and register its operations.
            //
            // - If the callee is a `RequestRoutine` → QC block
            // - If the callee is a `LocalRoutine`   → QL block
            //
            // We extract the body of the callee region and include all its operations
            // until and including the final `netqasm::ReturnOp`.
            if (llvm::isa<qoalahost::CallOp>(op)) {
                qoalahost::CallOp callOp = llvm::cast<qoalahost::CallOp>(op);

                mlir::Operation *calleeFunc = nullptr;
                if (auto symRef = callOp.getCalleeAttr().dyn_cast<mlir::SymbolRefAttr>()) {
                    calleeFunc = mlir::SymbolTable::lookupNearestSymbolFrom(moduleOp, symRef);
                }

                if (!calleeFunc || calleeFunc->getNumRegions() == 0 || calleeFunc->getRegion(0).empty()) {
                    mlir::emitError(op->getLoc(), "CallOp target is not a valid function with body");
                    std::vector<std::unique_ptr<MILPBlock>> emptyVec;
                    return std::pair<mlir::LogicalResult, std::vector<std::unique_ptr<MILPBlock>>>(mlir::failure(),
                                                                                                   std::move(emptyVec));
                }

                if (calleeFunc && llvm::isa<netqasm::RequestRoutineOp>(calleeFunc)) {
                    blockType = BlockType::QC;
                } else if (llvm::isa<netqasm::LocalRoutineOp>(calleeFunc)) {
                    blockType = BlockType::QL;
                }

                std::unique_ptr<MILPBlock> blockPtr = std::make_unique<MILPBlock>(blockId, blockType, &block);
                blockPtr->predecessors = preds;
                blockPtr->dependencies = deps;
                blockPtr->prevComm = prevComm;
                blockPtr->prevEnt = prevEnt;
                std::string opId = blockId + "::" + std::to_string(opIndex++);
                blockPtr->addOperation(opId, op);

                // Sanity check: ensure the region ends with a netqasm::ReturnOp.
                // This guarantees the semantic completeness of the routine block.
                // If no return is found, we abort the pass.
                bool foundReturn = false;
                mlir::Region &calleeRegion = calleeFunc->getRegion(0);
                for (mlir::Block &calleeBlock: calleeRegion) {
                    for (mlir::Operation &calleeOp: calleeBlock) {
                        std::string opId = blockId + "::" + std::to_string(opIndex++);
                        blockPtr->addOperation(opId, &calleeOp);
                        if (llvm::isa<netqasm::ReturnOp>(&calleeOp)) {
                            foundReturn = true;
                            break;
                        }
                    }
                    if (foundReturn)
                        break;
                }

                if (!foundReturn) {
                    mlir::emitError(op->getLoc(), "Callee did not end with netqasm::ReturnOp");
                    std::vector<std::unique_ptr<MILPBlock>> emptyVec;
                    return std::pair<mlir::LogicalResult, std::vector<std::unique_ptr<MILPBlock>>>(mlir::failure(),
                                                                                                   std::move(emptyVec));
                }

                milpBlocks.push_back(std::move(blockPtr));
                continue;
            }

            // --- Case 2: Comm Ops ---
            // These are classified as CC blocks. We register only the first operation,
            // and assume the block ends with a termination (not included in MILPBlock).
            if (llvm::isa<qoalahost::SendIntsOp>(op) || llvm::isa<qoalahost::RecvIntsOp>(op) ||
                llvm::isa<qoalahost::SendFloatsOp>(op) || llvm::isa<qoalahost::RecvFloatsOp>(op)) {
                std::unique_ptr<MILPBlock> blockPtr = std::make_unique<MILPBlock>(blockId, BlockType::CC, &block);
                blockPtr->predecessors = preds;
                blockPtr->dependencies = deps;
                blockPtr->prevComm = prevComm;
                blockPtr->prevEnt = prevEnt;
                std::string opId = blockId + "::" + std::to_string(opIndex++);
                blockPtr->addOperation(opId, op);
                milpBlocks.push_back(std::move(blockPtr));
                continue;
            }

            // --- Case 3: Generic CL ---
            // We treat all other blocks as CC-type and record their operations
            // until a terminal operation (e.g., NopTOp or ReturnOp) is reached.
            // If no useful operations are found (e.g., only BlkMeta + Return), we skip it.
            std::unique_ptr<MILPBlock> blockPtr = std::make_unique<MILPBlock>(blockId, BlockType::CL, &block);
            blockPtr->predecessors = preds;
            blockPtr->dependencies = deps;
            blockPtr->prevComm = prevComm;
            blockPtr->prevEnt = prevEnt;

            for (; it != block.end(); ++it) {
                mlir::Operation *innerOp = &*it;
                if (llvm::isa<qoalahost::NopTOp>(innerOp) || llvm::isa<qoalahost::ReturnOp>(innerOp))
                    break;
                std::string opId = blockId + "::" + std::to_string(opIndex++);
                blockPtr->addOperation(opId, innerOp);
            }

            if (blockPtr->operations.empty()) {
                LLVM_DEBUG(llvm::dbgs() << "[MILP] Skipping block with no useful ops: " << blockId << "\n");
                continue;
            }

            milpBlocks.push_back(std::move(blockPtr));
        }

        LLVM_DEBUG({
            llvm::dbgs() << "[MILP] Finished MILP block construction:\n";
            for (const std::unique_ptr<MILPBlock> &blkPtr: milpBlocks) {
                const MILPBlock &blk = *blkPtr;
                llvm::dbgs() << " - Block " << blk.id << " (type=" << static_cast<int>(blk.type) << ")\n";
                for (const MILPOperation &milpOp: blk.operations) {
                    llvm::dbgs() << "     * " << milpOp.id << " → " << milpOp.op->getName().getStringRef() << "\n";
                }
            }
        });

        return {mlir::success(), std::move(milpBlocks)};
    }

} // namespace qoala::analysis::reordering
