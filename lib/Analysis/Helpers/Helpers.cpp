#include "Analysis/Helpers/Helpers.h"

#include "mlir/Dialect/Arith/IR/Arith.h"

// include generated "dispatcher" of the operation interface
#include "Analysis/Helpers/GenericInterfaces.h"
#include "Analysis/Helpers/GenericInterfaces.cpp.inc"

using namespace mlir;

namespace qoala::helpers {
    /* Implementation of the null types converter */
    NullTypeConverter::NullTypeConverter(MLIRContext *ctx) {
        // For all types, we map it to the same instance
        this->addConversion([](const Type type) { return type; });
    }

    raw_ostream &operator<<(raw_ostream &os, const PrintInterface &printable) {
        printable.print(os);
        return os;
    }
} // namespace qoala::helpers
