#include "Analysis/Helpers/Helpers.h"
#include "Analysis/Helpers/NetQASMInterfaces.h"
#include "Analysis/Helpers/QoalaHostInterfaces.h"
#include "Analysis/QoalaHost/RemoteIDs.h"
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Dialect/QRemote/QRemote.h"

#define DEBUG_TYPE "add-remote-id-placeholders"

using namespace mlir;
using namespace qoala::dialects;

namespace qoala::analysis::remoteids {
    LogicalResult addRemoteIDs(ModuleOp &module) {
        const auto mainFuncs = module.getOps<qoalahost::MainFuncOp>();
        const auto remoteDeclarations = module.getOps<qremote::RemoteOp>();
        if (mainFuncs.empty()) {
            module->emitError("No main QoalaHost main function found in the module");
            return failure();
        }
        if (remoteDeclarations.empty()) {
            // No remote declarations to process... nothing to do here
            return success();
        }

        qoalahost::MainFuncOp mainFunc = *mainFuncs.begin();
        Block &firstBlock = mainFunc.front();
        assert(firstBlock.getOperations().size() == 1 && "Trying to insert remote ID palceholders on"
                                                         "a non \"empty\" first block.");

        // Analyze the "usages" of the QRemote.remote op (see the example in the translation)
        for (auto remoteDecl : remoteDeclarations) {
            // We analyze the usages of remoteOp. If there are classical comms, add classical socket declaration
            // with the remote. If there are quantum ops, also add EPR socket declaration with the remote.
            const StringRef usedRemoteName = remoteDecl.getName();
            bool classicalCommUse = false;
            bool quantumCommUse = false;
            bool *classCommUsePtr = &classicalCommUse;
            bool *quantumCommUsePtr = &quantumCommUse;

            module.walk([&](helpers::UsesRemoteInterface commOp) -> WalkResult {
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
            // Insert a placeholder for each of the csocket/qsocket found
            OpBuilder builder(&firstBlock, firstBlock.begin());
            builder.create<qoalahost::RemoteIDRefOp>(
                    firstBlock.begin()->getLoc(), FlatSymbolRefAttr::get(remoteDecl.getSymNameAttr()),
                    builder.getBoolAttr(classicalCommUse), builder.getBoolAttr(quantumCommUse));
        }

        return success();
    }
} // namespace qoala::analysis::remoteids
