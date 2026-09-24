#include "llvm/Support/raw_ostream.h"

#include "Analysis/Helpers/Print.h"

// include generated "dispatcher" of the operation interface
// NOTE: clang-format is disabled for the following includes.
// These must remain in this specific order, changing it breaks the build.
// clang-format off
#include "Analysis/Helpers/GenericInterfaces.h"
#include "Analysis/Helpers/GenericInterfaces.cpp.inc"
// clang-format on

namespace qoala::helpers::print {
    llvm::raw_ostream &operator<<(llvm::raw_ostream &os, const PrintInterface &printable) {
        printable.print(os);
        return os;
    }
} // namespace qoala::helpers::print
