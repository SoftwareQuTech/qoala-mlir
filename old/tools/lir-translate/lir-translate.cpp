#include "mlir/InitAllTranslations.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Translation.h"

#include "Target/lir/ConvertToText.h"

using namespace mlir;

int main(int argc, char **argv)
{
    registerAllTranslations();
    registerToLirTranslation();
    return failed(mlirTranslateMain(argc, argv, "MLIR Quantum Translation Tool"));
}
