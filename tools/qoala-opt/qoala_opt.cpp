#include "mlir/InitAllDialects.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Pass/PassRegistry.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

#include "Dialect/Qnet/Passes.h"
#include "Dialect/Qnet/QnetDialect.h"

#include "Dialect/Qmem/QmemDialect.h"

// Since the lowering pass is part of this opt tool,
// we need to also link the libraries of the LIR dialect
#include "Dialect/lir/LirDialect.h"

#include "Dialect/Netqasm/NetqasmDialect.h"

// And, of course, we also need the libraries of the lowering pass itself
#include "Conversion/QnetToQmem/QnetToQmem.h"

int main(int argc, char **argv) {
    mlir::DialectRegistry registry;
    // We register all the default dialects from MLIR
    registerAllDialects(registry);
    // We register all the dialects we are implementing
    registry.insert<qoala::dialects::qnet::QnetDialect>();
    registry.insert<qoala::dialects::qmem::QmemDialect>();
    registry.insert<mlir::lir::LirDialect>();
    registry.insert<mlir::netqasm::NetqasmDialect>();

    // We also register all the passes from MLIR
    mlir::registerAllPasses();
    // And also the passes from Qnet
    mlir::registerQnetPasses();
    // And the pass that converts Qnet to Qmem dialect
    mlir::registerQnetToQmemPasses();

    mlir::registerViewOpGraphPass();

    mlir::MLIRContext context;

    mlir::PassManager pm(&context);

    return failed(
        mlir::MlirOptMain(argc, argv, "Qoala IR optimizer\n", registry));
}
