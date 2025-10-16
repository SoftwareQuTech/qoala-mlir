// UNSUPPORTED: true
// Reason: see the comment on the qoala_wrapper_2 request routine op
// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s
// CHECK: META_START
// CHECK-NEXT: name: test_qubit_tracking
// CHECK-NEXT: parameters: Bob_id
// CHECK-NEXT: csockets: 0 -> Bob
// CHECK-NEXT: epr_sockets: 0 -> Bob
// CHECK-NEXT: META_END
// CHECK-NEXT: ^b[[BLOCK0:.*]] { type = QL, predecessors = [] }
// This call does not yield a result, because __qoala_wrapper0 "uses 0" and "keeps 0"
// CHECK-NEXT: %[[HOST_REG0:.*]] = run_request() : __qoala_wrapper0
// CHECK: ^b[[BLOCK1:.*]] { type = QL, predecessors = [b0] }
// This call does not required an argument, since __qoala_wrapper "uses 0"
// CHECK-NEXT: %[[HOST_REG1:.*]] = run_request() : __qoala_wrapper1
// CHECK: ^b[[BLOCK2:.*]] { type = QC, predecessors = [b0] }
// CHECK-NEXT: %[[HOST_REG2:.*]] = run_subroutine() : __qoala_wrapper2
// CHECK: ^b[[BLOCK3:.*]] { type = QC, predecessors [b0, b1] }
// CHECK-NEXT: %[[HOST_REG3:.*]] = run_subroutine() : __qoala_wrapper2
// CHECK: ^b[[BLOCK4:.*]] { type = CL, predecessors = [b2, b3] }
// CHECK-NEXT: %[[HOST_REG4:.*]] = add_cval_c(%[[HOST_REG2]], %[[HOST_REG3]])
// CHECK-NEXT: return_value(%[[HOST_REG4]])

// WARNING: This using the same request routine multiple times does not work
// correctly.
// Subroutines *must declare* the qubits they use. In this sense, we *cannot*
// reuse the same code for different qubits, despite in MLIR we can pass different
// qubit values to the same subroutine.
// A potential solution for this would be to duplicate the MC code before resolving the
// placeholder operands, so we can resolve them multiple times when called.
// This solution is left for future work.
// CHECK: SUBROUTINE __qoala_wrapper2
// Used qubits should not be marked as parameters
// CHECK-NEXT: params:
// CHECK-NEXT: returns: m0
// CHECK-NEXT: uses: [[QUBIT0:.*]]
// CHECK-NEXT: keeps:
// CHECK-NEXT: request:
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: set [[QUBIT_REG0:.*]] [[QUBIT0]]
// CHECK-NEXT: meas [[QUBIT_REG0]] [[M_REG0:.*]]
// CHECK-NEXT: store [[M_REG0]] @output[0]
// CHECK-NEXT: NETQASM_END

//CHECK: REQUEST __qoala_wrapper0
// CHECK-NEXT: callback_type: sequential
// CHECK-NEXT: callback:
// CHECK-NEXT: return_vars: m0
// CHECK-NEXT: remote_id: {Bob_id}
// CHECK-NEXT: epr_socket_id: 0
// CHECK-NEXT: num_pairs: 1
// CHECK-NEXT: virt_ids: all 0
// CHECK-NEXT: timeout: 1000
// CHECK-NEXT: fidelity: 1.000000e+00
// CHECK-NEXT: typ: create_keep
// CHECK-NEXT: role: create

//CHECK: REQUEST __qoala_wrapper1
// CHECK-NEXT: callback_type: sequential
// CHECK-NEXT: callback:
// CHECK-NEXT: return_vars: m0
// CHECK-NEXT: remote_id: {Bob_id}
// CHECK-NEXT: epr_socket_id: 0
// CHECK-NEXT: num_pairs: 1
// CHECK-NEXT: virt_ids: all 1
// CHECK-NEXT: timeout: 1000
// CHECK-NEXT: fidelity: 1.000000e+00
// CHECK-NEXT: typ: create_keep
// CHECK-NEXT: role: create

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  // This routine returns directly a qubit "pointer"
  netqasm.request_routine @__qoala_wrapper0() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.eprs %0  {remote = @Bob}
    netqasm.return %0 : i32
  }
  // This routine returns directly a qubit "pointer"
  netqasm.request_routine @__qoala_wrapper1() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.eprs %0  {remote = @Bob}
    netqasm.return %0 : i32
  }
  // This routine returns directly a qubit measurement<
  netqasm.local_routine @__qoala_wrapper2(%qubit: i32) -> i1 {
    %0 = netqasm.measure %qubit : i1
    netqasm.return %0 : i1
  }
  qoalahost.main_func @test_qubit_tracking() -> i1{
  %0 = qoalahost.call @__qoala_wrapper0() : () -> i32
  ^bb1:
    %1 = qoalahost.call @__qoala_wrapper1() : () -> i32
  ^bb2:
    %2 = qoalahost.call @__qoala_wrapper2(%0) : (i32) -> i1
  ^bb3:
    %3 = qoalahost.call @__qoala_wrapper2(%1) : (i32) -> i1
  ^bb4:
    %result = arith.addi %2, %3 : i1
    qoalahost.return %result : i1
  }
}
