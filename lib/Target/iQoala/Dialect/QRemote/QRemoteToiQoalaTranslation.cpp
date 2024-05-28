#include "mlir/IR/Operation.h"
#include "Target/iQoala/QoalaTranslationInterface.h"
#include "Target/iQoala/Dialect/QRemote/QRemoteToiQoalaTranslation.h"
#include "Target/iQoala/ModuleTranslation.h"
#include "llvm/ADT/TypeSwitch.h"
#include "llvm/Support/Debug.h"

#include "Dialect/QRemote/QRemote.h"

#define DEBUG_TYPE "qremote-translation"

using namespace qoala::dialects::qremote;
using namespace qoala::iqoala;

static LogicalResult translateRemoteDeclaration(RemoteOp &remoteOp, ModuleTranslation &moduleTranslation) {
    moduleTranslation.addRemoteDeclaration(remoteOp.getSymNameAttr());
    LLVM_DEBUG(llvm::dbgs() << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
    return success();
}

static LogicalResult translateQRemoteOperation(Operation *operation, ModuleTranslation &moduleTranslation) {
    LLVM_DEBUG(llvm::dbgs() << "******** Translating op '" << operation->getName() << "' *********\n");
    return llvm::TypeSwitch<Operation *, LogicalResult>(operation)
            .Case([&](RemoteOp op)-> LogicalResult {
                return translateRemoteDeclaration(op, moduleTranslation);
            })
            .Default([](Operation *op) -> LogicalResult {
                return op->emitOpError("Unknown way to translate a QRemote operation to iQoala: '") << *op << "'\n";
            });
}

namespace qoala::translate {
    class QRemoteToiQoalaTranslationInterface : public QoalaTranslationDialectInterface {
    public:
        using QoalaTranslationDialectInterface::QoalaTranslationDialectInterface;
        LogicalResult convertOperation(Operation *op, ModuleTranslation &moduleTranslation) const final {
            return translateQRemoteOperation(op, moduleTranslation);
        }

    };

    void registerQRemoteToiQoalaTranslations(DialectRegistry &registry) {
        registry.insert<QRemoteDialect>();
        registry.addExtension(+[](MLIRContext *ctx, QRemoteDialect *dialect) {
            dialect->addInterfaces<QRemoteToiQoalaTranslationInterface>();
        });
    }
}