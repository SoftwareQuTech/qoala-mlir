#include "mlir/IR/Operation.h"
#include "Target/iQoala/QoalaTranslationInterface.h"
#include "Target/iQoala/Dialect/NetQASM/NetQASMToiQoalaTranslation.h"
#include "llvm/Support/Debug.h"

#include "Dialect/NetQASM/NetQASM.h"

using namespace qoala::dialects;
using namespace qoala::iqoala;

static LogicalResult translateNetQASMOperation(Operation *operation) {
    // TODO - Implement this method
    operation->dump();
    llvm::dbgs() << "******** NetQASM - AQUI! *********\n";
    return success();
}

namespace qoala::translate {
    class NetQASMToiQoalaTranslationInterface : public QoalaTranslationDialectInterface {
    public:
        using QoalaTranslationDialectInterface::QoalaTranslationDialectInterface;
        LogicalResult convertOperation(Operation *op, ModuleTranslation &moduleTranslation) const final {
            return translateNetQASMOperation(op);
        }

    };

    void registerNetQASMToiQoalaTranslations(DialectRegistry &registry) {
        registry.insert<netqasm::NetQASMDialect>();
        registry.addExtension(+[](MLIRContext *ctx, netqasm::NetQASMDialect *dialect) {
            dialect->addInterfaces<NetQASMToiQoalaTranslationInterface>();
        });
    }
}