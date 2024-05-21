#include "mlir/IR/Operation.h"
#include "Translation/QoalaTranslationInterface.h"
#include "Target/iQoala/Dialect/QRemote/QRemoteToiQoalaTranslation.h"
#include "llvm/Support/Debug.h"

#include "Dialect/QRemote/QRemote.h"

using namespace qoala::dialects;

static LogicalResult translateQRemoteOperation(Operation *operation) {
    // TODO - Implement this method
    llvm::dbgs() << "******** AQUI! *********\n";
    return success();
}

namespace qoala::translate {
    class QRemoteToiQoalaTranslationInterface : public QoalaTranslationDialectInterface {
    public:
        using QoalaTranslationDialectInterface::QoalaTranslationDialectInterface;
        LogicalResult
        convertOperation(Operation *op) const final {
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