#include "mlir/InitAllTranslations.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Tools/mlir-translate/MlirTranslateMain.h"

using namespace mlir;

/* Dummy declarations of these *unused* (in the translate tool) options
 * so the linking of qoala-translate passes correctly.
 */
namespace qoala::options {
    bool qoalaOptUnoptimize = false;
    uint32_t qoalaOptSingleGateDuration = 0;
    uint32_t qoalaOptTwoGateDuration = 0;
    uint32_t qoalaOptLatency = 0;
    uint32_t qoalaOptLinkDuration = 0;
    uint32_t qoalaOptHostInstrTime = 0;
    uint32_t qoalaOptHostPeerLatency = 0;
    uint32_t qoalaOptQNosInstrTime = 0;
    uint32_t qoalaOptQubitLifetime = 0;
    bool qoalaOptGroupEntReqs = false;
    uint32_t qoalaOptProgramHorizon = 0;
} // namespace qoala::options

namespace qoala::translate {
    void registerToiQoalaTranslations();
}

int main(int argc, char **argv) {
    registerAllTranslations();
    // We register the translations of to the iQoala format
    qoala::translate::registerToiQoalaTranslations();
    return failed(mlirTranslateMain(argc, argv, "Qoala Translation Testing Tool"));
}
