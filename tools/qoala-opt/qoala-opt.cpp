#include "mlir/InitAllDialects.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Pass/PassRegistry.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

#include "Dialect/QNet/Passes.h"
#include "Dialect/QNet/QNetDialect.h"

#include "Dialect/QMem/QMemDialect.h"
#include "Dialect/NetQASM/NetQASMDialect.h"
#include "Dialect/QoalaHost/QoalaHostDialect.h"

// Since the lowering pass is part of this opt tool,
// we need to also link the libraries of the LIR dialect
#include "Dialect/lir/LirDialect.h"

// And, of course, we also need the libraries of the lowering pass itself
#include "Conversion/QNetToQMem/QNetToQMem.h"
#include "Conversion/QMemToQoalaHost/QMemToQoalaHost.h"

int main(int argc, char **argv) {
    mlir::DialectRegistry registry;
    // We register all the default dialects from MLIR
    registerAllDialects(registry);
    // We register all the dialects we are implementing
    registry.insert<qoala::dialects::qnet::QNetDialect>();
    registry.insert<qoala::dialects::qmem::QMemDialect>();
    registry.insert<qoala::dialects::netqasm::NetQASMDialect>();
    registry.insert<qoala::dialects::qoalahost::QoalaHostDialect>();
    registry.insert<mlir::lir::LirDialect>();

    // We also register all the passes from MLIR
    mlir::registerAllPasses();
    // And also the passes from QNet
    mlir::registerQNetPasses();
    // And the pass that converts QNet to QMem dialect
    mlir::registerQNetToQMemPasses();
    // And the pass that converts QMem to QoalaHost dialect
    mlir::registerQMemToQoalaHostPasses();

    mlir::registerViewOpGraphPass();

    mlir::MLIRContext context;

    mlir::PassManager pm(&context);

    return failed(
        mlir::MlirOptMain(argc, argv, "Qoala IR optimizer\n", registry));
}
