#include "Target/iQoala/Dialect/QRemote/QRemoteToiQoalaTranslation.h"

#include "Dialect/NetQASM/NetQASM.h"
#include "Dialect/QRemote/QRemote.h"
#include "Target/iQoala/ModuleTranslation.h"
#include "llvm/ADT/TypeSwitch.h"
#include "llvm/Support/Casting.h"
#include "mlir/IR/Operation.h"

#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "qremote-translation"

using namespace mlir;
using namespace qoala::translate;
using namespace qoala::dialects::qremote;
using namespace qoala::dialects::netqasm;

static LogicalResult translateRemoteDeclaration(RemoteOp &remoteOp, const ModuleTranslation *moduleTranslation) {
    // We analyze the usages of remoteOp. If there are classical comms, add classical socket declaration
    // with the remote. If there are quantum ops, also add EPR socket declaration with the remote.
    const auto remoteOpUses = remoteOp->getUses();
    const bool classicalCommUse = std::any_of(remoteOpUses.begin(), remoteOpUses.end(), [](const OpOperand &opOperand) {
        Operation *use = opOperand.getOwner();
        return llvm::isa<qoala::helpers::ClassicalCommInterface>(use);
    });
    const bool quantumCommUse = std::any_of(remoteOpUses.begin(), remoteOpUses.end(), [](const OpOperand &opOperand) {
        Operation *use = opOperand.getOwner();
        return llvm::isa<EprsOp, EprsMeasureOp>(use);
    });
    const bool addedRemote =
            moduleTranslation->addRemoteDeclaration(remoteOp.getSymNameAttr(), classicalCommUse, quantumCommUse);
    if (!addedRemote) {
        remoteOp.emitWarning("Remote with name '" + remoteOp.getName() + "' is not used. " +
                             "The remote reference was not added as an iQoala parameter.");
    }
    return success();
}

static LogicalResult translateQRemoteOperation(Operation *operation, const ModuleTranslation *moduleTranslation) {
    return llvm::TypeSwitch<Operation *, LogicalResult>(operation)
            .Case([&](RemoteOp op) -> LogicalResult { return translateRemoteDeclaration(op, moduleTranslation); })
            .Default([](Operation *op) -> LogicalResult {
                return op->emitOpError("Unknown way to translate a QRemote operation to iQoala: '") << *op << "'\n";
            });
}

namespace qoala::translate {
    LogicalResult QRemoteToiQoalaTranslation::convertOperation(Operation *op,
                                                               ModuleTranslation *moduleTranslation) const {
        return translateQRemoteOperation(op, moduleTranslation);
    }
} // namespace qoala::translate
