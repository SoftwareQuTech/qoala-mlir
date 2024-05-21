#include "mlir/IR/Operation.h"
#include "Translation/QoalaTranslationInterface.h"
#include "Target/iQoala/Dialect/Tensor/TensorToiQoalaTranslation.h"
#include "llvm/Support/Debug.h"

#include "mlir/Dialect/Tensor/IR/Tensor.h"

static LogicalResult translateNetQASMOperation(Operation *operation) {
    // TODO - Implement this method
    operation->dump();
    llvm::dbgs() << "******** Tensor - AQUI! *********\n";
    return success();
}

namespace qoala::translate {
    class TensorToiQoalaTranslationInterface : public QoalaTranslationDialectInterface {
    public:
        using QoalaTranslationDialectInterface::QoalaTranslationDialectInterface;
        LogicalResult
        convertOperation(Operation *op) const final {
            return translateNetQASMOperation(op);
        }

    };

    void registerTensorToiQoalaTranslations(DialectRegistry &registry) {
        registry.insert<tensor::TensorDialect>();
        registry.addExtension(+[](MLIRContext *ctx, tensor::TensorDialect *dialect) {
            dialect->addInterfaces<TensorToiQoalaTranslationInterface>();
        });
    }
}