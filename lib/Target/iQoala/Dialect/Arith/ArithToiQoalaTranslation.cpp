#include "mlir/IR/Operation.h"
#include "Target/iQoala/QoalaTranslationInterface.h"
#include "Target/iQoala/Dialect/Arith/ArithToiQoalaTranslation.h"
#include "llvm/Support/Debug.h"

#include "mlir/Dialect/Arith/IR/Arith.h"

#define DEBUG_TYPE "arith-translation"

using namespace mlir;
using namespace qoala::iqoala;

static LogicalResult translateNetQASMOperation(Operation *operation) {
    // TODO - Implement this dispatcher
    LLVM_DEBUG(llvm::dbgs() << "******** Translating op '" << operation->getName() << "' *********\n");
    return success();
}

namespace qoala::translate {
    class ArithToiQoalaTranslationInterface : public QoalaTranslationDialectInterface {
    public:
        using QoalaTranslationDialectInterface::QoalaTranslationDialectInterface;
        LogicalResult convertOperation(Operation *op, ModuleTranslation &moduleTranslation) const final {
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