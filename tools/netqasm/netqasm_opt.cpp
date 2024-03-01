#include "mlir/IR/AsmState.h"
#include "mlir/InitAllDialects.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Pass/PassRegistry.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

#include "Dialect/netqasm/Netqasm.h"
#include "Dialect/netqasm/NetqasmDialect.h"

int main(int argc, char **argv) {
  mlir::DialectRegistry registry;
  registerAllDialects(registry);
  registry.insert<mlir::netqasm::NetqasmDialect>();

  mlir::registerAllPasses();

  mlir::registerViewOpGraphPass();

  mlir::MLIRContext context;

  mlir::PassManager pm(&context);

  return failed(
      mlir::MlirOptMain(argc, argv, "Netqasm dialect optimizer\n", registry));
}
