#include "mlir/IR/AsmState.h"
#include "mlir/InitAllDialects.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Support/MlirOptMain.h"

#include "Dialect/hir/Passes.h"
#include "Dialect/hir/HirDialect.h"
#include "Dialect/clir/CLirDialect.h"
#include "Dialect/qlir/QLirDialect.h"
#include "Dialect/lir/LirDialect.h"
#include "Dialect/lir/Passes.h"
#include "Conversion/HirToLir/Passes.h"
#include "Conversion/LirToSplitLir/Passes.h"

int main(int argc, char **argv)
{
    mlir::DialectRegistry registry;
    registerAllDialects(registry);
    registry.insert<mlir::hir::HirDialect>();
    registry.insert<mlir::clir::CLirDialect>();
    registry.insert<mlir::qlir::QLirDialect>();
    registry.insert<mlir::lir::LirDialect>();

    mlir::registerAllPasses();
    mlir::registerHirPasses();
    mlir::registerLirPasses();
    mlir::registerHirToLirConversionPasses();
    mlir::registerLirToSplitLirConversionPasses();

    return failed(
        mlir::MlirOptMain(argc, argv, "HIR dialect driver\n", registry));
}
