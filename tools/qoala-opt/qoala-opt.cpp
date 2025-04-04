#include "mlir/InitAllDialects.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Pass/PassRegistry.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

#include "Dialect/QNet/Passes.h"
#include "Dialect/QNet/QNetDialect.h"

#include "Dialect/QMem/Passes.h"
#include "Dialect/QMem/QMemDialect.h"

#include "Dialect/QoalaHost/Passes.h"
#include "Dialect/QoalaHost/QoalaHostDialect.h"

#include "Dialect/NetQASM/NetQASMDialect.h"
#include "Dialect/QoalaHost/QoalaHostDialect.h"
#include "Dialect/QRemote/QRemoteDialect.h"

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
    registry.insert<qoala::dialects::qremote::QRemoteDialect>();

    // We also register all the passes from MLIR
    mlir::registerAllPasses();
    // And also the passes from QMem, QNet and QoalaHost
    qoala::analysis::registerQNetPasses();
    qoala::analysis::registerQMemPasses();
    qoala::analysis::registerQoalaHostPasses();
    // And the pass that lowers Qoala HIR to MIR (conversion from QNet to QMem dialect)
    qoala::conversion::registerQoalaHIRToQoalaMIRPasses();
    // And the pass that converts QMem to QoalaHost dialect
    qoala::conversion::registerQoalaMIRToQoalaLIRPasses();

    mlir::registerViewOpGraphPass();

    mlir::MLIRContext context;

    mlir::PassManager pm(&context);

    return mlir::asMainReturnCode(
        mlir::MlirOptMain(argc, argv, "Qoala IR optimizer\n", registry));
}
