#include "mlir/IR/Operation.h"
#include "Target/iQoala/QoalaTranslationInterface.h"
#include "Target/iQoala/Dialect/Builtin/BuiltinToiQoalaTranslation.h"
#include "llvm/Support/Debug.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/BuiltinDialect.h"

#define DEBUG_TYPE "builtin-translation"

using namespace qoala::iqoala;

static LogicalResult translateBuiltinOperation(Operation *operation) {
    // TODO - Implement this dispatcher
    LLVM_DEBUG(llvm::dbgs() << "******** Translating op '" << operation->getName() << "' *********\n");
    return success();
}

namespace qoala::translate {
    class BuiltinToiQoalaTranslationInterface : public QoalaTranslationDialectInterface {
    public:
        using QoalaTranslationDialectInterface::QoalaTranslationDialectInterface;
        LogicalResult convertOperation(Operation *op, ModuleTranslation &moduleTranslation) const final {
            return translateBuiltinOperation(op);
        }

    };

    void registerBuiltinToiQoalaTranslations(DialectRegistry &registry) {
        registry.addExtension(+[](MLIRContext *ctx, BuiltinDialect *dialect) {
            dialect->addInterfaces<BuiltinToiQoalaTranslationInterface>();
        });
    }
}