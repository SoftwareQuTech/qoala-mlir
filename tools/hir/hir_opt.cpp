#include "mlir/IR/AsmState.h"
#include "mlir/InitAllDialects.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Pass/PassRegistry.h"

#include "Dialect/hir/HirDialect.h"
#include "Dialect/hir/Hir.h"

#include "Dialect/hir/Passes.h"

int main(int argc, char **argv)
{
    mlir::DialectRegistry registry;
    registerAllDialects(registry);
    registry.insert<mlir::hir::HirDialect>();

    mlir::registerAllPasses();
    mlir::registerHirPasses();

    mlir::registerViewOpGraphPass();

    mlir::MLIRContext context;

    mlir::PassManager pm(&context);
    // pm.addPass(mlir::createHirCheckLinearPass());

    return failed(
        mlir::MlirOptMain(argc, argv, "Qoala HIR optimizer\n", registry));
}
