#ifndef QOALA_MLIR_HELPERS_H
#define QOALA_MLIR_HELPERS_H

#include "mlir/IR/Operation.h"

using namespace mlir;

namespace qoala::helpers {
    void printOperation(Operation *op);
    void printRegion(Region &region);
    void printBlock(Block &block);

    struct IdentRAII {
        int &indent;
        IdentRAII(int &indent) : indent(indent) {}
        ~IdentRAII() { --indent; }
    };

    void resetIndent();
    IdentRAII pushIndent();
    llvm::raw_ostream &printIndent();
}

#endif //QOALA_MLIR_HELPERS_H
