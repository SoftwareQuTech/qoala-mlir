#ifndef MODULETRANSLATION_H
#define MODULETRANSLATION_H

#include "Target/iQoala/Export.h"
#include "Target/iQoala/QoalaTranslationInterface.h"
#include "mlir/IR/Operation.h"

using namespace mlir;
using namespace qoala;

namespace qoala::translate {
    class ModuleTranslation {
        friend std::unique_ptr<iqoala::Module>
        qoala::translate::translateModuleToiQoala(Operation *module, iqoala::iQoalaContext &iQoalaContext,
                                                  llvm::StringRef name);
    private:
        ModuleTranslation(Operation *module,
                          std::unique_ptr<iqoala::Module> iQoalaModule);
        ~ModuleTranslation() = default;
        LogicalResult convertOperation(Operation &op, bool recordInsertions = false);

        /* Class fields */
        Operation *mlirModule;
        std::unique_ptr<iqoala::Module> iQoalaModule;
        iqoala::QoalaTranslationInterfaces iface;
        // TODO - Define the public functions that we need to place in this class
    };
}

#endif //MODULETRANSLATION_H
