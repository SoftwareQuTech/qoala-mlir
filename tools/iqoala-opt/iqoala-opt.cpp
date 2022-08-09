#include "mlir/IR/AsmState.h"
#include "mlir/InitAllDialects.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Support/MlirOptMain.h"
#include "mlir/Pass/PassManager.h"

#include "Dialect/iqoala/iQoalaDialect.h"
#include "Dialect/iqoala/iQoala.h"

int main(int argc, char **argv)
{
    mlir::DialectRegistry registry;
    registerAllDialects(registry);
    registry.insert<mlir::iqoala::iQoalaDialect>();

    mlir::registerAllPasses();

    mlir::registerViewOpGraphPass();

    mlir::MLIRContext context;
    mlir::PassManager pm(&context);
    pm.addPass(mlir::createCanonicalizerPass());

    return failed(
        mlir::MlirOptMain(argc, argv, "Toy dialect driver\n", registry));
}
