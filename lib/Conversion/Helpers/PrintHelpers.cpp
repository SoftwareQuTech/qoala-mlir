#include "Conversion/Helpers/Helpers.h"

using namespace mlir;
using namespace qoala::helpers;

namespace qoala::helpers::print {
    uint32_t indent;

    void resetIndent() { indent = 0; }

    IdentRAII pushIndent() { return IdentRAII(++indent); }

    llvm::raw_ostream &printIndent() {
        for (uint32_t i = 0; i < indent; ++i) {
            llvm::outs() << "  ";
        }
        return llvm::outs();
    }

    void printRegion(Region &region) {
        // A region does not hold anything by itself other than a list of blocks.
        printIndent() << "Region with " << region.getBlocks().size() << " blocks:\n";
        auto indent = pushIndent();
        for (Block &block : region.getBlocks()) {
            printBlock(block);
        }
    }

    void printBlock(Block &block) {
        // Print the block intrinsics properties (basically: argument list)
        printIndent() << "Block with " << block.getNumArguments() << " arguments, " << block.getNumSuccessors()
                      << " successors, and "
                      // Note, this `.size()` is traversing a linked-list and is O(n).
                      << block.getOperations().size() << " operations\n";

        // Block main role is to hold a list of Operations: let's recurse.
        auto indent = pushIndent();
        for (Operation &op : block.getOperations()) {
            printOperation(&op);
        }
    }

    void printOperation(Operation *op) {
        // Print the operation itself and some of its properties
        printIndent() << "visiting op: '" << op->getName() << "' with " << op->getNumOperands() << " operands and "
                      << op->getNumResults() << " results\n";
        // Print the operation attributes
        if (!op->getAttrs().empty()) {
            printIndent() << op->getAttrs().size() << " attributes:\n";
            for (NamedAttribute attr : op->getAttrs()) {
                printIndent() << " - '" << attr.getName().getValue() << "' : '" << attr.getValue() << "'\n";
            }
        }

        // Recurse into each of the regions attached to the operation.
        printIndent() << " " << op->getNumRegions() << " nested regions:\n";
        auto indent = pushIndent();
        for (Region &region : op->getRegions()) {
            printRegion(region);
        }
    }
} // namespace qoala::helpers::print
