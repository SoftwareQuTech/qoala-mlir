#include "mlir/InitAllDialects.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Pass/PassRegistry.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

#include "Dialect/QNet/Passes.h"
#include "Dialect/QNet/QNetDialect.h"

#include "Dialect/QMem/Passes.h"
#include "Dialect/QMem/QMemDialect.h"

#include "Dialect/NetQASM/NetQASMDialect.h"
#include "Dialect/QoalaHost/QoalaHostDialect.h"

#include "Conversion/QoalaHIRToQoalaMIR/QoalaHIRToQoalaMIR.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h"

int main(int argc, char **argv) {
    mlir::DialectRegistry registry;
    // We register all the default dialects from MLIR
    registerAllDialects(registry);
    // We register all the dialects we are implementing
    registry.insert<qoala::dialects::qnet::QNetDialect>();
    registry.insert<qoala::dialects::qmem::QMemDialect>();
    registry.insert<qoala::dialects::netqasm::NetQASMDialect>();
    registry.insert<qoala::dialects::qoalahost::QoalaHostDialect>();

    // We also register all the passes from MLIR
    mlir::registerAllPasses();
    // And also the passes from QNet
    mlir::registerQNetPasses();
    mlir::registerQMemPasses();
    // And the pass that lowers Qoala HIR to MIR (conversion from QNet to QMem dialect)
    mlir::registerQoalaHIRToQoalaMIRPasses();
    // And the pass that converts QMem to QoalaHost dialect
    mlir::registerQoalaMIRToQoalaLIRPasses();

    mlir::registerViewOpGraphPass();

    mlir::MLIRContext context;

    mlir::PassManager pm(&context);

    return failed(
        mlir::MlirOptMain(argc, argv, "Qoala IR optimizer\n", registry));
}
