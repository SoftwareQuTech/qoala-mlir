#ifndef QOALA_MLIR_PRINT_H
#define QOALA_MLIR_PRINT_H

// #include "llvm/Support/raw_ostream.h"

namespace qoala::helpers::print {
    /**
     * Simple interface that defines a "print" method
     */
    class PrintInterface {
    public:
        PrintInterface() = default;
        virtual ~PrintInterface() = default;
        virtual void print(llvm::raw_ostream &os) const = 0;
    };

    llvm::raw_ostream &operator<<(llvm::raw_ostream &os, const PrintInterface &printable);
} // namespace qoala::helpers::print
#endif // QOALA_MLIR_PRINT_H
