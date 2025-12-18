#include "Analysis/QoalaHost/RemoteIDs.h"
#include "Dialect/QoalaHost/QoalaHost.h"

using namespace mlir;
using namespace qoala::dialects::qoalahost;

namespace qoala::analysis::remoteids {
    LogicalResult addRemoteIDs(ModuleOp &module) {
        // TODO - Big skeleton
        //  1. Create a new block at the beginning of the main function (
        //  2. Analyze the "usages of the QRemote.remote op (see the example in the translation)
        //  3. Insert a placeholder for each of the csocket/qsocket found
        const auto mainFuncs = module.getOps<MainFuncOp>();
        if (mainFuncs.empty()) {
            module->emitError("No main QoalaHost main function found in the module");
            return failure();
        }

        MainFuncOp mainFunc = *mainFuncs.begin();

        return success();
    }

} // namespace qoala::analysis::remoteids

