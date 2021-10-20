#include "mlir/IR/AsmState.h"
#include "mlir/InitAllDialects.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Support/MlirOptMain.h"

#include "Dialect/hir/HirDialect.h"

int main(int argc, char **argv)
{
    mlir::registerAllPasses();

    mlir::DialectRegistry registry;
    // registerAllDialects(registry);
    registry.insert<mlir::hir::HirDialect>();

    return failed(
        mlir::MlirOptMain(argc, argv, "HIR dialect driver\n", registry));
}
