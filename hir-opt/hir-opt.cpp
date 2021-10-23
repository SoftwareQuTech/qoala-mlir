#include "mlir/IR/AsmState.h"
#include "mlir/InitAllDialects.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Support/MlirOptMain.h"

#include "Dialect/hir/Passes.h"
#include "Dialect/hir/HirDialect.h"
#include "Dialect/lir/LirDialect.h"

int main(int argc, char **argv)
{
    mlir::DialectRegistry registry;
    registerAllDialects(registry);
    registry.insert<mlir::hir::HirDialect>();
    registry.insert<mlir::lir::LirDialect>();

    mlir::registerAllPasses();
    mlir::registerHirPasses();

    return failed(
        mlir::MlirOptMain(argc, argv, "HIR dialect driver\n", registry));
}
