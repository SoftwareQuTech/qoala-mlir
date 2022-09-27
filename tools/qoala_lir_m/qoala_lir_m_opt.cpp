#include "mlir/IR/AsmState.h"
#include "mlir/InitAllDialects.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Support/MlirOptMain.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Pass/PassRegistry.h"

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

    // pm acts (by default) on the `builtin.module` op.
    mlir::PassManager pm(&context);

    // applyPassManagerCLOptions(pm);

    // OpPassManager &netqasmFuncPM = pm.nest<mlir::qoala_lir_m::NetqasmFuncOp>();
    // netqasmFuncPM.addPass(createLirMGenericPass());

    mlir::registerPass([]() -> std::unique_ptr<::mlir::Pass>
                       { return createLirMGenericPass(); });

    // PassRegistration<LirMGenericPass>();

    // registerP

    // PassPipelineRegistration<>(
    //     "generic-pass", "", [](OpPassManager &pm)
    //     { pm.addPass(createLirMGenericPass()); });

    return failed(
        mlir::MlirOptMain(argc, argv, "Qoala LIR-M optimizer\n", registry));
}
