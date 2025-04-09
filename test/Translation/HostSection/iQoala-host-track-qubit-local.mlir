// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s
// CHECK: META START
// CHECK-NEXT: name: test_qubit_tracking
// CHECK-NEXT: parameters: Bob_id
// CHECK-NEXT: csockets: 0 -> Bob
// CHECK-NEXT: epr_sockets: 0 -> Bob
// CHECK-NEXT: META END
// CHECK-NEXT: b[[BLOCK0:.*]] { type = CL }
// CHECK-NEXT: %[[HOST_REG0:.*]] = assign_cval() : 0
// CHECK: b[[BLOCK1:.*]] { type = CL }
// This call does not yield a result, because __qoala_wrapper0 "uses 0" and "keeps 0"
// CHECK-NEXT: run_subroutine() : __qoala_wrapper0
// CHECK: b[[BLOCK2:.*]] { type = CL }
// This call does not required an argument, since __qoala_wrapper "uses 0"
// CHECK-NEXT: %[[HOST_REG2:.*]] = run_subroutine() : __qoala_wrapper1
// CHECK: b[[BLOCK3:.*]] { type = CL }
// CHECK-NEXT: %[[HOST_REG3:.*]] = add_cval_c(%[[HOST_REG0]], %[[HOST_REG2]])

// CHECK: SUBROUTINE __qoala_wrapper0
// CHECK-NEXT: params:
// CHECK-NEXT: returns: m0
// CHECK-NEXT: uses: [[QUBIT0:.*]]
// CHECK-NEXT: keeps: [[QUBIT0]]
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: set [[QUBIT_REG0:.*]] [[QUBIT0]]
// CHECK-NEXT: init [[QUBIT_REG0]]
// CHECK-NEXT: store [[QUBIT_REG0]] @output[0]
// CHECK-NEXT: NETQASM_END

// CHECK: SUBROUTINE __qoala_wrapper1
// Used qubits should not be marked as parameters
// CHECK-NEXT: params:
// CHECK-NEXT: returns: m0
// CHECK-NEXT: uses: [[QUBIT0]]
// CHECK-NEXT: keeps:
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: set [[QUBIT_REG0:.*]] [[QUBIT0]]
// CHECK-NEXT: meas [[QUBIT_REG0]] [[M_REG0:.*]]
// CHECK-NEXT: store [[M_REG0]] @output[0]
// CHECK-NEXT: NETQASM_END

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  // This routine returns directly a qubit "pointer"
  netqasm.local_routine @__qoala_wrapper0() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.return %0 : i32
  }
  // This routine returns directly a qubit measurement<
  netqasm.local_routine @__qoala_wrapper1(%qubit: i32) -> i1 {
    %0 = netqasm.measure %qubit : i1
    netqasm.return %0 : i1
  }
  qoalahost.main_func @test_qubit_tracking() {
    %cst = arith.constant 0 : i1
    qoalahost.nop_term
  ^bb1:
    %0 = qoalahost.call @__qoala_wrapper0() : () -> i32
  ^bb2:
    %1 = qoalahost.call @__qoala_wrapper1(%0) : (i32) -> i1
  ^bb3:
    %2 = arith.addi %cst, %1 : i1
    qoalahost.return
  }
}
