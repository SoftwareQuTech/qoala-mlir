#include "mlir/IR/AsmState.h"
#include "mlir/InitAllDialects.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Support/MlirOptMain.h"

#include "Dialect/toy/ToyDialect.h"

int main(int argc, char **argv)
{
    mlir::DialectRegistry registry;
    registerAllDialects(registry);
    registry.insert<mlir::toy::ToyDialect>();

    mlir::registerAllPasses();

    mlir::registerViewOpGraphPass();

    return failed(
        mlir::MlirOptMain(argc, argv, "Toy dialect driver\n", registry));
}
