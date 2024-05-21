#include "mlir/IR/Operation.h"
#include "Translation/QoalaTranslationInterface.h"
#include "Target/iQoala/Dialect/Arith/ArithToiQoalaTranslation.h"
#include "llvm/Support/Debug.h"

#include "mlir/Dialect/Arith/IR/Arith.h"

static LogicalResult translateNetQASMOperation(Operation *operation) {
    // TODO - Implement this method
    operation->dump();
    llvm::dbgs() << "******** Arith - AQUI! *********\n";
    return success();
}

namespace qoala::translate {
    class ArithToiQoalaTranslationInterface : public QoalaTranslationDialectInterface {
    public:
        using QoalaTranslationDialectInterface::QoalaTranslationDialectInterface;
        LogicalResult
        convertOperation(Operation *op) const final {
            return translateNetQASMOperation(op);
        }

    };

    void registerArithToiQoalaTranslations(DialectRegistry &registry) {
        registry.insert<arith::ArithDialect>();
        registry.addExtension(+[](MLIRContext *ctx, arith::ArithDialect *dialect) {
            dialect->addInterfaces<ArithToiQoalaTranslationInterface>();
        });
    }
}