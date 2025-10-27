#include "Analysis/Helpers/Helpers.h"

#include "mlir/Dialect/Arith/IR/Arith.h"

// include generated "dispatcher" of the operation interface
// NOTE: clang-format is disabled for the following includes.
// These must remain in this specific order, changing it breaks the build.
// clang-format off
#include "Analysis/Helpers/GenericInterfaces.h"
#include "Analysis/Helpers/GenericInterfaces.cpp.inc"
// clang-format on

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
