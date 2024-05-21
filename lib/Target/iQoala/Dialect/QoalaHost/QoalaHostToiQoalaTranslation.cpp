#include "mlir/IR/Operation.h"
#include "Translation/QoalaTranslationInterface.h"
#include "Target/iQoala/Dialect/QoalaHost/QoalaHostToiQoalaTranslation.h"
#include "llvm/Support/Debug.h"

#include "Dialect/QoalaHost/QoalaHost.h"

using namespace qoala::dialects;

static LogicalResult translateQoalaHostOperation(Operation *operation) {
    // TODO - Implement this method
    operation->dump();
    llvm::dbgs() << "******** QoalaHost - AQUI! *********\n";
    return success();
}

namespace qoala::translate {
    class QoalaHostToiQoalaTranslationInterface : public QoalaTranslationDialectInterface {
    public:
        using QoalaTranslationDialectInterface::QoalaTranslationDialectInterface;

        LogicalResult
        convertOperation(Operation *op) const final {
            return translateQoalaHostOperation(op);
        }

    };

    void registerQoalaHostToiQoalaTranslations(DialectRegistry &registry) {
        registry.insert<qoalahost::QoalaHostDialect>();
        registry.addExtension(+[](MLIRContext *ctx, qoalahost::QoalaHostDialect *dialect) {
            dialect->addInterfaces<QoalaHostToiQoalaTranslationInterface>();
        });
    }
}