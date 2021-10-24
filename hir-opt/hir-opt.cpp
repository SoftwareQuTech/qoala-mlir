#include "mlir/IR/AsmState.h"
#include "mlir/InitAllDialects.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Support/MlirOptMain.h"

#include "Dialect/hir/Passes.h"
#include "Dialect/hir/HirDialect.h"
#include "Dialect/clir/CLirDialect.h"
#include "Dialect/qlir/QLirDialect.h"
#include "Dialect/lircommon/LirCommonDialect.h"
#include "Conversion/HirToLir/Passes.h"

int main(int argc, char **argv)
{
    mlir::DialectRegistry registry;
    registerAllDialects(registry);
    registry.insert<mlir::hir::HirDialect>();
    registry.insert<mlir::clir::CLirDialect>();
    registry.insert<mlir::qlir::QLirDialect>();
    registry.insert<mlir::lircommon::LirCommonDialect>();

    mlir::registerAllPasses();
    mlir::registerHirPasses();
    mlir::registerHirToLirConversionPasses();

    return failed(
        mlir::MlirOptMain(argc, argv, "HIR dialect driver\n", registry));
}
