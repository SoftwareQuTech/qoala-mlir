#include "mlir/InitAllTranslations.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Tools/mlir-translate/MlirTranslateMain.h"

using namespace mlir;

namespace qoala::translate {
    void registerToiQoalaTranslations();
}

int main(int argc, char **argv) {
  registerAllTranslations();
  qoala::translate::registerToiQoalaTranslations();
  return failed(mlirTranslateMain(argc, argv, "Qoala Translation Testing Tool"));
}
