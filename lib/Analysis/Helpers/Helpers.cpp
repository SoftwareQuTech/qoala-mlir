#include "Analysis/Helpers/Helpers.h"

using namespace mlir;

namespace qoala::helpers {
    /* Implementation of the null types converter */
    NullTypeConverter::NullTypeConverter(MLIRContext *ctx) {
        // For all types, we map it to the same instance
        this->addConversion([](Type type) { return type; });
    }

    raw_ostream &qoala::helpers::operator<<(raw_ostream &os, const PrintInterface &printable) {
        printable.print(os);
        return os;
    }
}
