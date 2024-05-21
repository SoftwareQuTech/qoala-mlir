#ifndef MODULETRANSLATION_H
#define MODULETRANSLATION_H

#include "Target/iQoala/Export.h"

using namespace mlir;

namespace qoala::translate {
    class ModuleTranslation {
        friend std::unique_ptr<qoala::iqoala::Module>
        translateModuleToiQoala(Operation *module, qoala::iqoala::iQoalaContext &iQoalaContext,
                                llvm::StringRef name);
    public:
        // TODO - Define the public functions that we need to place in this class
    };
}

#endif //MODULETRANSLATION_H
