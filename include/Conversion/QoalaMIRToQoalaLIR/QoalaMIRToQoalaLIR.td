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
    let constructor = "::mlir::createQoalaMIRToQoalaLIRPass()";
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
    let constructor = "::mlir::createLowerQMemToQoalaHostPass()";
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
    let constructor = "::mlir::createLowerQMemToNetQASMPass()";
    // Since we lower QMem to NetQASM, we need to depend on all those dialects to create
    // valid MLIR
    let dependentDialects = ["::mlir::func::FuncDialect",
                             "::qoala::dialects::qmem::QMemDialect",
                             "::qoala::dialects::netqasm::NetQASMDialect"];
}

#endif /* QMEM_TO_QOALAHOST_PASS */
