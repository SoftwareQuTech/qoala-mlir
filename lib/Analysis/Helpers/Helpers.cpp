#include "Analysis/Helpers/Helpers.h"
#include <string>

using namespace mlir;
using namespace qoala::helpers;

/* Implementation of the null types converter */
qoala::helpers::NullTypeConverter::NullTypeConverter(MLIRContext *ctx) {
    // For all types, we map it to the same instance
    this->addConversion([](Type type) { return type; });
}
