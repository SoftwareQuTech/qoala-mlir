// Reason: See comment on "__qoala_wrapper1"
// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s
// CHECK: META START
// CHECK-NEXT: name: test_call_local_routine
// CHECK-NEXT: parameters: Bob_id
// CHECK-NEXT: csockets: 0 -> Bob
// CHECK-NEXT: epr_sockets: 0 -> Bob
// CHECK-NEXT: META END
// CHECK: b[[BLOCK0:.*]] { type = CL }
// This call does not yield a result, because __qoala_wrapper0 "uses 0" and "keeps 0"
// CHECK-NEXT: run_subroutine() : __qoala_wrapper0
// CHECK: b[[BLOCK1:.*]] { type = CL }
// This call does not required an argument, since __qoala_wrapper "uses 0"
// CHECK-NEXT: %[[HOST_REG1:.*]] = run_subroutine() : __qoala_wrapper1
// CHECK: b[[BLOCK2:.*]] { type = CL }

// CHECK: SUBROUTINE __qoala_wrapper0
// CHECK-NEXT: params: {{[[:space:]]}}
// Since we are matching the newline char in the last check, we need to start matching
// on the same line!
// CHECK-SAME: returns: {{[[:space:]]}}
// CHECK-SAME: uses: [[QUBIT0:.*]]
// Since "meas" is considered a "qfree", this subroutine does not keep the qubit 0
// CHECK-NEXT: keeps: [[QUBIT0]]
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: set [[QUBIT_REG0:.*]] [[QUBIT0]]
// CHECK-NEXT: init [[QUBIT_REG0]]
// CHECK-NEXT: NETQASM_END

// This test case expects to declare using physical qubit 0, and keeping none.
// Currently this is not happening, since it is still not possible
// to figure out that the argument passed to this local routine is
// a qubit.
// A solution for this will be implemented together when mapping the
// call function in the qoalahost section:
// * When translating a call, we will perform a small data flow analysis
//   in the body. If the local routine returns a value and the returned value
//   can be traced back to a qalloc operation, then we will map the result of the
//   qoalahost.call operation in the qoalahost body to the physical qubit id.
// * We will later use this information when processing other calls; if passing
//   a value to a local routine that is mapped to a physical qubit, then we will
//   add the physical id in the "uses" section, and map the argument value as
//   one of the physical qubits used.
// CHECK: SUBROUTINE __qoala_wrapper1
// CHECK-NEXT: params: {{[[:space:]]}}
// CHECK-SAME: returns: m0
// CHECK-NEXT: uses: [[QUBIT0]]
// Since "meas" is considered a "qfree", this subroutine does not keep the qubit 0
// CHECK-NEXT: keeps: {{[[:space:]]}}
// CHECK-SAME: NETQASM_START
// CHECK-NEXT: set [[QREG0:.*]] [[QUBIT0]]
// CHECK-NEXT: meas [[QREG0]] [[MREG0:.*]]
// CHECK-NEXT: store [[MREG0]] @output[0]
// CHECK-NEXT: NETQASM_END

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @__qoala_wrapper0() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.return %0 : i32
  }
  netqasm.local_routine @__qoala_wrapper1(%arg0: i32) -> i1 {
    %0 = netqasm.measure %arg0 : i1
    netqasm.return %0: i1
  }
  qoalahost.main_func @test_call_local_routine() {
    // Note: there is an implicit "^bb0" not-rendered block declaration here
    // so this "call" operation is the one and only operation of the
    // first block of the main function
    %0 = qoalahost.call @__qoala_wrapper0() : () -> i32
    ^bb1:
        %1 = qoalahost.call @__qoala_wrapper1(%0) : (i32) -> i1
    ^bb2:
        qoalahost.return
  }
}
