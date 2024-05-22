#include "mlir/IR/Operation.h"
#include "Target/iQoala/QoalaTranslationInterface.h"
#include "Target/iQoala/Dialect/QRemote/QRemoteToiQoalaTranslation.h"
#include "llvm/Support/Debug.h"

#include "Dialect/QRemote/QRemote.h"

using namespace qoala::dialects;
using namespace qoala::iqoala;

static LogicalResult translateQRemoteOperation(Operation *operation) {
    // TODO - Implement this method
    operation->dump();
    llvm::dbgs() << "******** QRemote - AQUI! *********\n";
    return success();
}

namespace qoala::translate {
    class QRemoteToiQoalaTranslationInterface : public QoalaTranslationDialectInterface {
    public:
        using QoalaTranslationDialectInterface::QoalaTranslationDialectInterface;
        LogicalResult convertOperation(Operation *op, ModuleTranslation &moduleTranslation) const final {
            return translateQRemoteOperation(op);
        }

    };

    void registerQMemToiQoalaTranslations(DialectRegistry &registry) {
        registry.insert<qremote::QRemoteDialect>();
        registry.addExtension(+[](MLIRContext *ctx, qremote::QRemoteDialect *dialect) {
            dialect->addInterfaces<QRemoteToiQoalaTranslationInterface>();
        });
    }
}