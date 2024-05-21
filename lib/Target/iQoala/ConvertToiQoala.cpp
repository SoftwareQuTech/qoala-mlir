#include "mlir/Tools/mlir-translate/Translation.h"
#include "Target/iQoala/QoalaTranslations.h"
#include "Target/iQoala/Export.h"

/* Include the dialects that we insert in the dialect registry */
/* First, the dialects that come from MLIR */
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Tensor/IR/Tensor.h"
/* Second, the dialects defined by us */
#include "Dialect/QoalaHost/QoalaHost.h"
#include "Dialect/NetQASM/NetQASM.h"
#include "Dialect/QRemote/QRemote.h"

using namespace mlir;
using namespace qoala::dialects;

namespace qoala::translate {
    void registerToiQoalaTranslations() {
        [[maybe_unused]]
        TranslateFromMLIRRegistration registration(
                "mlir-to-iqoala", "Translate MLIR to iQoala",
                [](Operation *op, raw_ostream &output) -> LogicalResult {
                    // TODO - Double check that we are passing the right arguments:
                    //  It seems we need to pass the operation (the full module), but also a "context" object,
                    //  which will aid the process of exporting the MLIR
                    qoala::iqoala::iQoalaContext iQoalaContext;
                    auto iQoalaModule = translateModuleToiQoala(op, iQoalaContext);
                    if (!iQoalaModule) {
                        return failure();
                    }

                    return success();
                },
                [](DialectRegistry &registry) {
                    registerAllQoalaTranslations(registry);
                    registerAllQoalaSupportTranslations(registry);
                });
    }
} // namespace mlir