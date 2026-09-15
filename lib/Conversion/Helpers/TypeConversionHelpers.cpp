#include "llvm/Support/raw_ostream.h"
#include "mlir/IR/MLIRContext.h"

#include "Analysis/Helpers/Conversion.h"
#include "Conversion/Helpers/Helpers.h"

using namespace mlir;

namespace qoala::helpers::conversion {
    /* Implementation of the null types converter */
    NullTypeConverter::NullTypeConverter(MLIRContext *ctx) {
        // For all types, we map it to the same instance
        this->addConversion([](const Type type) { return type; });
    }
} // namespace qoala::helpers::conversion
