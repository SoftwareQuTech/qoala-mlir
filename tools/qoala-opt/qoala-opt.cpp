#include "llvm/Support/CommandLine.h"
#include "mlir/InitAllDialects.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Pass/PassRegistry.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

#include "Dialect/QNet/Passes.h"
#include "Dialect/QNet/QNetDialect.h"

#include "Dialect/Helpers/MIRToLIRHelperPasses.h"
#include "Dialect/QMem/QMemDialect.h"

#include "Dialect/QoalaHost/Passes.h"
#include "Dialect/QoalaHost/QoalaHostDialect.h"

#include "Dialect/NetQASM/NetQASMDialect.h"
#include "Dialect/QRemote/QRemoteDialect.h"
#include "Dialect/QoalaHost/QoalaHostDialect.h"

#include "Conversion/QoalaHIRToQoalaMIR/QoalaHIRToQoalaMIR.h"
#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.h"

#include "Tools/QoalaOpt.h"

bool qoala::options::qoalaOptUnoptimize = false;
static opt<bool, /*ExternalStorage=*/true> qoalaOptUnoptimizeOption(
        "qoala-opt-unoptimize",
        desc("Wether to run the passes to unoptimize the program. Useful to compare worst vs best "
             "case implementations of programs."),
        ReallyHidden, location(qoala::options::qoalaOptUnoptimize));

uint32_t qoala::options::qoalaOptSingleGateDuration = 10;
static opt<uint32_t, /*ExternalStorage=*/true>
        qoalaOptSingleGateDurationOption("qoala-opt-single-gate-duration",
                                         desc("Time taken by a single gate operation."), NotHidden,
                                         location(qoala::options::qoalaOptSingleGateDuration));

float qoala::options::qoalaOptSingleGateError = 0.01;
static opt<float, /*ExternalStorage=*/true>
        qoalaOptSingleGateErrorOption("qoala-opt-single-gate-error", desc("Error rate of a single gate operation."),
                                      NotHidden, location(qoala::options::qoalaOptSingleGateError));

uint32_t qoala::options::qoalaOptTwoGateDuration = 50;
static opt<uint32_t, /*ExternalStorage=*/true>
        qoalaOptTwoGateDurationOption("qoala-opt-two-gate-duration", desc("Time taken by a double gate operation."),
                                      NotHidden, location(qoala::options::qoalaOptTwoGateDuration));

float qoala::options::qoalaOptDualGateError = 0.05;
static opt<float, /*ExternalStorage=*/true>
        qoalaOptDualGateErrorOption("qoala-opt-dual-gate-error", desc("Error rate of a double gate operation."),
                                    NotHidden, location(qoala::options::qoalaOptDualGateError));

uint32_t qoala::options::qoalaOptLatency = 100;
static opt<uint32_t, /*ExternalStorage=*/true> qoalaOptLatencyOption("qoala-opt-latency", NotHidden,
                                                                     location(qoala::options::qoalaOptLatency));

uint32_t qoala::options::qoalaOptLinkDuration = 1000;
static opt<uint32_t, /*ExternalStorage=*/true>
        qoalaOptLinkDurationOption("qoala-opt-link-duration", desc("Time taken to generate entanglement."), NotHidden,
                                   location(qoala::options::qoalaOptLinkDuration));
uint32_t qoala::options::qoalaOptHostInstrTime = 1;
static opt<uint32_t, /*ExternalStorage=*/true>
        qoalaOptHostInstrTimeOption("qoala-opt-host-instr-time", desc("Time taken by a CL operation on the QNPU."),
                                    NotHidden, location(qoala::options::qoalaOptHostInstrTime));
uint32_t qoala::options::qoalaOptHostPeerLatency = 1;
static opt<uint32_t, /*ExternalStorage=*/true> qoalaOptHostPeerLatencyOption(
        "qoala-opt-host-peer-latency",
        desc("Latency of treating a classical communication operation upon receiving a message3."), NotHidden,
        location(qoala::options::qoalaOptHostPeerLatency));
uint32_t qoala::options::qoalaOptQNosInstrTime = 1;
static opt<uint32_t, /*ExternalStorage=*/true>
        qoalaOptQNosInstrTimeOption("qoala-opt-qnos-instr-time",
                                    desc("Time taken by a classical instruction executed on the QNPU."), NotHidden,
                                    location(qoala::options::qoalaOptQNosInstrTime));
uint32_t qoala::options::qoalaOptQubitLifetime = 500;
static opt<uint32_t, /*ExternalStorage=*/true>
        qoalaOptQubitLifetimeOption("qoala-opt-qubit-lifetime", desc("Lifetime of a qubit."), NotHidden,
                                    location(qoala::options::qoalaOptQubitLifetime));

bool qoala::options::qoalaOptGroupEntReqs = false;
static opt<bool, /*ExternalStorage=*/true>
        qoalaOptGroupEntReqsOption("qoala-opt-group-ent-reqs", desc("Whether to group entanglement requests or not."),
                                   NotHidden, location(qoala::options::qoalaOptGroupEntReqs));

uint32_t qoala::options::qoalaOptProgramHorizon = 0;
static opt<uint32_t, /*ExternalStorage=*/true>
        qoalaOptProgramHorizonOption("qoala-opt-program-horizon",
                                     desc("Program horizon i.e, the maximum time the program should take to execute."),
                                     NotHidden, location(qoala::options::qoalaOptProgramHorizon));

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
    qoala::analysis::registerMIRToLIRHelpersPasses();
    qoala::analysis::registerQoalaHostPasses();
    // And the pass that lowers Qoala HIR to MIR (conversion from QNet to QMem dialect)
    qoala::conversion::registerQoalaHIRToQoalaMIRPasses();
    // And the pass that converts QMem to QoalaHost dialect
    qoala::conversion::registerQoalaMIRToQoalaLIRPasses();

    mlir::registerViewOpGraphPass();

    mlir::MLIRContext context;

    mlir::PassManager pm(&context);

    return mlir::asMainReturnCode(mlir::MlirOptMain(argc, argv, "Qoala IR optimizer\n", registry));
}
