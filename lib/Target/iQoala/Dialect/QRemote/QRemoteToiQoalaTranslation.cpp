#include "Target/iQoala/Dialect/QRemote/QRemoteToiQoalaTranslation.h"

#include "Analysis/Helpers/NetQASMInterfaces.h"
#include "Analysis/Helpers/QoalaHostInterfaces.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "Dialect/QRemote/QRemote.h"
#include "Target/iQoala/ModuleTranslation.h"
#include "llvm/ADT/TypeSwitch.h"
#include "llvm/Support/Casting.h"
#include "mlir/IR/Operation.h"

#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "qremote-translation"

using namespace llvm;
using namespace mlir;
using namespace qoala::translate;
using namespace qoala::dialects;

static LogicalResult translateRemoteDeclaration(qremote::RemoteOp &remoteOp,
                                                const ModuleTranslation *moduleTranslation) {
    // We analyze the usages of remoteOp. If there are classical comms, add classical socket declaration
    // with the remote. If there are quantum ops, also add EPR socket declaration with the remote.
    auto module = remoteOp->getParentOfType<ModuleOp>();
    const StringRef usedRemoteName = remoteOp.getName();
    bool classicalCommUse = false;
    bool quantumCommUse = false;
    bool *classCommUsePtr = &classicalCommUse;
    bool *quantumCommUsePtr = &quantumCommUse;

    module.walk([&](qoala::helpers::UsesRemoteInterface commOp) -> WalkResult {
        if (isa<qoalahost::ifaces::ClassicalCommInterface>(commOp.getOperation())) {
            if (commOp.getUsedRemoteName() == usedRemoteName) {
                *classCommUsePtr = true;
            }
        }
        if (isa<netqasm::ifaces::EntangledQubitOp>(commOp.getOperation())) {
            if (commOp.getUsedRemoteName() == usedRemoteName) {
                *quantumCommUsePtr = true;
            }
        }
        return WalkResult::advance();
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
            .Case([&](qremote::RemoteOp op) -> LogicalResult {
                return translateRemoteDeclaration(op, moduleTranslation);
            })
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
