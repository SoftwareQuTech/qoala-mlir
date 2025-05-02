#ifndef QMEM_TO_QOALAHOST_PASS
#define QMEM_TO_QOALAHOST_PASS

include "mlir/Pass/PassBase.td"

//===----------------------------------------------------------------------===//
// General Transformation Pass from Qoala MIR to Qoala LIR
// This pass is basically a wrapper to call all another passes to allow
// *full* conversion from Qoala MIR to Qoala LIR
//===----------------------------------------------------------------------===//

def QoalaMIRToQoalaLIR : Pass<"lower-qoala-mir-to-lir", "mlir::ModuleOp"> {
    let summary = "Lower Qoala MIR to Qoala LIR";
    let description = [{
        This pass fully lowers Qoala MIR to Qoala LIR. In this sense, it is equivalent to
        running "qmem-functionize", "lower-qmem-to-qoalahost" and "lower-qmem-to-netqasm"
        passes in that order. Considering that, this pass is intended to be a syntax sugar
        for final users of the qoala compiler.
    }];
    /*
     *IMPORTANT: To use this option, the help suggests that is can be used as an argument
     * of the "opt" tool, i.e. "--use-simple-functionize". However, THIS WILL NOT WORK.
     * To correctly use this option, it needs to be invoked _as an argument of the pass option_:
     * i.e. "--lower-qoala-mir-to-lir=use-simple-functionize"
     * For integer values, we can simply use --lower-qoala-mir-to-lir=max-ops-per-group=5
     */
    let options = [
        Option<"useSimpleFunctionize", "use-simple-functionize", "bool", /*default=*/"false",
               "WARNING: Use simple functionize; use a single quantum operation in a function wrapper.">,
        Option<"useSCCP", "use-sccp", "bool", /*default=*/"false",
               "Use an SCCP pass to propagate constants inside nested blocks in the main function.">,
        Option<"maxOpsPerGroup", "max-ops-per-group", "uint32_t", /*default=*/"0",
               "Set the max number of operations to place per group. This option only affects the per-qubit grouping functionization">
    ];
    // Since we lower QMem to NetQASM and QoalaHost, we need to depend on all those dialects to create
    // valid MLIR
    let dependentDialects = ["::mlir::func::FuncDialect",
                             "::qoala::dialects::qmem::QMemDialect",
                             "::qoala::dialects::qoalahost::QoalaHostDialect",
                             "::qoala::dialects::netqasm::NetQASMDialect",
                             "::qoala::dialects::qremote::QRemoteDialect"];
}

//===----------------------------------------------------------------------===//
// Transformation Pass from QMem to QoalaHost
//===----------------------------------------------------------------------===//

def LowerQMemToQoalaHost : Pass<"lower-qmem-to-qoalahost", "mlir::ModuleOp"> {
    let summary = "Lower all the instances of qmem operations that can be lowered to the qoalahost dialect";
    // Since we lower QMem to QoalaHost, we need to depend on all those dialects to create
    // valid MLIR
    let dependentDialects = ["::mlir::func::FuncDialect",
                             "::qoala::dialects::qmem::QMemDialect",
                             "::qoala::dialects::qoalahost::QoalaHostDialect",
                             "::qoala::dialects::qremote::QRemoteDialect"];
}

//===----------------------------------------------------------------------===//
// Transformation Pass from QMem to NetQASM
//===----------------------------------------------------------------------===//

def LowerQMemToNetQASM : Pass<"lower-qmem-to-netqasm", "mlir::ModuleOp"> {
    let summary = "Lower all the instances of qmem operations that can be lowered to the NetQASM dialect";
    // Since we lower QMem to NetQASM, we need to depend on all those dialects to create
    // valid MLIR
    let dependentDialects = ["::mlir::func::FuncDialect",
                             "::qoala::dialects::qmem::QMemDialect",
                             "::qoala::dialects::netqasm::NetQASMDialect"];
}

//===----------------------------------------------------------------------===//
// Transformation Pass from QMem to QRemote
//===----------------------------------------------------------------------===//

def LowerQMemToQRemote : Pass<"lower-qmem-to-qremote", "mlir::ModuleOp"> {
    let summary = "Lower all the instances of qmem operations that can be lowered to the QRemote dialect";
    // Since we lower QMem to QRemote, we need to depend on all those dialects to create
    // valid MLIR
    let dependentDialects = ["::qoala::dialects::qremote::QRemoteDialect"];
}

#endif /* QMEM_TO_QOALAHOST_PASS */
