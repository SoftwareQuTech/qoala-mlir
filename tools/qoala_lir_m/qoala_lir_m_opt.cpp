#include "mlir/IR/AsmState.h"
#include "mlir/InitAllDialects.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Support/MlirOptMain.h"
#include "mlir/Pass/PassManager.h"

#include "Dialect/qoala_lir_m/QoalaLirMDialect.h"
#include "Dialect/qoala_lir_m/QoalaLirM.h"

#include "Dialect/qoala_lir_m/Transforms/QoalaLirMTransforms.h"

int main(int argc, char **argv)
{
    mlir::DialectRegistry registry;
    registerAllDialects(registry);
    registry.insert<mlir::qoala_lir_m::QoalaLirMDialect>();

    mlir::registerAllPasses();

    mlir::registerViewOpGraphPass();

    mlir::MLIRContext context;
    mlir::PassManager pm(&context);
    // pm.addPass(mlir::createCanonicalizerPass());

    pm.addPass(createLirMGenericPass());

    return failed(
        mlir::MlirOptMain(argc, argv, "Toy dialect driver\n", registry));
}
