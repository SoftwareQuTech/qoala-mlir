#include "mlir/IR/AsmState.h"
#include "mlir/InitAllDialects.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Support/MlirOptMain.h"

#include "Dialect/hir/Passes.h"
#include "Dialect/hir/HirDialect.h"
#include "Dialect/clir/CLirDialect.h"
#include "Dialect/qlir/QLirDialect.h"
#include "Dialect/lircommon/LirCommonDialect.h"
#include "Dialect/lircommon/Passes.h"
#include "Conversion/HirToLirCommon/Passes.h"
#include "Conversion/LirCommonToLir/Passes.h"

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
    mlir::registerLirCommonPasses();
    mlir::registerHirToLirCommonConversionPasses();
    mlir::registerLirCommonToLirConversionPasses();

    return failed(
        mlir::MlirOptMain(argc, argv, "HIR dialect driver\n", registry));
}
