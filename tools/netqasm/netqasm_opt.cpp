#include "mlir/IR/AsmState.h"
#include "mlir/InitAllDialects.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Support/MlirOptMain.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Pass/PassRegistry.h"

#include "Dialect/netqasm/NetqasmDialect.h"
#include "Dialect/netqasm/Netqasm.h"

int main(int argc, char **argv)
{
    mlir::DialectRegistry registry;
    registerAllDialects(registry);
    registry.insert<mlir::netqasm::NetqasmDialect>();

    mlir::registerAllPasses();

    mlir::registerViewOpGraphPass();

    mlir::MLIRContext context;

    mlir::PassManager pm(&context);

    return failed(
        mlir::MlirOptMain(argc, argv, "Qoala LIR-M optimizer\n", registry));
}
