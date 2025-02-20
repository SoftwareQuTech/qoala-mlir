#include "mlir/IR/Operation.h"
#include "Target/iQoala/QoalaTranslationInterface.h"
#include "Target/iQoala/Dialect/QRemote/QRemoteToiQoalaTranslation.h"
#include "Target/iQoala/ModuleTranslation.h"
#include "llvm/ADT/TypeSwitch.h"
#include "llvm/Support/Debug.h"

#include "Dialect/QRemote/QRemote.h"

#define DEBUG_TYPE "qremote-translation"

using namespace mlir;
using namespace qoala::dialects::qremote;

static LogicalResult translateRemoteDeclaration(RemoteOp &remoteOp, qoala::translate::ModuleTranslation &moduleTranslation) {
    moduleTranslation.addRemoteDeclaration(remoteOp.getSymNameAttr());
    return success();
}

static LogicalResult translateQRemoteOperation(Operation *operation, qoala::translate::ModuleTranslation &moduleTranslation) {
    return llvm::TypeSwitch<Operation *, LogicalResult>(operation)
            .Case([&](RemoteOp op)-> LogicalResult {
                return translateRemoteDeclaration(op, moduleTranslation);
            })
            .Default([](Operation *op) -> LogicalResult {
                return op->emitOpError("Unknown way to translate a QRemote operation to iQoala: '") << *op << "'\n";
            });
}

namespace qoala::translate {
    LogicalResult QRemoteToiQoalaTranslation::convertOperation(Operation *op, ModuleTranslation &moduleTranslation) const {
        return translateQRemoteOperation(op, moduleTranslation);
    }
}