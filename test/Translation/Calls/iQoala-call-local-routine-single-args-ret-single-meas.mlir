// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s
// CHECK: META START
// CHECK-NEXT: name: test_call_local_routine
// CHECK-NEXT: parameters: Bob
// CHECK-NEXT: csockets: 0 -> Bob
// CHECK-NEXT: epr_sockets: 0 -> Bob
// CHECK-NEXT: META END
// CHECK: b[[BLOCK0:.*]] { type = CL }
// CHECK: b[[BLOCK1:.*]] { type = CL }

//CHECK: SUBROUTINE __qoala_wrapper0
// CHECK-NEXT: params: {{[[:space:]]}}
// Since we are matching the newline char in the last check, we need to start matching
// on the same line!
// CHECK-SAME: returns: m0
// CHECK-NEXT: uses: [[QUBIT0:.*]]
// Since "meas" is considered a "qfree", this subroutine does not keep the qubit 0
// CHECK-NEXT: keeps: [[QUBIT0:.*]]
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: set [[QUBIT_REG0:.*]] [[QUBIT0]]
// CHECK-NEXT: init [[QUBIT_REG0]]
// CHECK-NEXT: store [[QUBIT_REG0]] @output[0]
// CHECK-NEXT: NETQASM_END

//CHECK: SUBROUTINE __qoala_wrapper1
// CHECK-NEXT: params: p0
// CHECK-NEXT: returns: m0
// CHECK-NEXT: uses: [[QUBIT0:.+]]
// Since "meas" is considered a "qfree", this subroutine does not keep the qubit 0
// CHECK-NEXT: keeps: {{[[:space:]]}}
// CHECK-SAME: NETQASM_START
// CHECK-NEXT: set [[CREG0:.*]] [[QUBIT0]]
// CHECK-NEXT: load [[RREG0:.*]] @input[[[CREG0]]]
// CHECK-NEXT: meas [[RREG0]] [[M_REG0:.*]]
// CHECK-NEXT: store [[M_REG0]] @output[0]
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
        %1 = qoalahost.call @__qoala_wrapper0(%0) : (i32) -> i1
    ^bb2:
        qoalahost.return
  }
}
