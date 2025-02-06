#include "mlir/Tools/mlir-translate/Translation.h"
#include "Target/iQoala/QoalaTranslations.h"
#include "Target/iQoala/Export.h"

using namespace mlir;

namespace qoala::translate {
    void registerToiQoalaTranslations() {
        /* Registration is "simple".
         * The created object receives 2 callbacks: "function" and "dialectRegistration"
         * - "dialectRegistration" is the second one, and it gets called when initializing the tool.
         *   This callback simply registers the respective dialects and the translation interfaces
         * - "function" is the "entry point" of the translation process, and gets called when starting
         *   the process of translation. This function should act as a "dispatcher" to fully translate
         *   all the nested operations into the iQoala format.
         */
        [[maybe_unused]]
        TranslateFromMLIRRegistration registration(
                "mlir-to-iqoala", "Translate MLIR to iQoala", // Command line arg, and description
                [](Operation *op, raw_ostream &output) -> LogicalResult {
                    // TODO - Double check that we are passing the right arguments:
                    //  It seems we need to pass the operation (the full module), but also a "context" object,
                    //  which will aid the process of exporting the MLIR
                    qoala::iqoala::iQoalaContext iQoalaContext;
                    auto iQoalaModule = translateModuleToiQoala(op, iQoalaContext);
                    if (!iQoalaModule) {
                        return failure();
                    }

                    // Print the module on the output stream
                    iQoalaModule->print(output);

                    return success();
                },
                [](DialectRegistry &registry) {
                    // Translation of "qoala dialects" to iQoala
                    registerAllQoalaTranslations(registry);
                    // Translations of "support dialects" (such as arith, func, and tensor) to iQoala
                    registerAllQoalaSupportTranslations(registry);
                });
    }
} // namespace mlir