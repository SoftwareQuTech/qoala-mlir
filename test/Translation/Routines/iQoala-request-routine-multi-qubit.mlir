// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s
// CHECK: META START
// CHECK-NEXT: name: test_call_request_routines
// CHECK-NEXT: parameters: Bob_id
// CHECK-NEXT: csockets: 0 -> Bob
// CHECK-NEXT: epr_sockets: 0 -> Bob
// CHECK-NEXT: META END
// CHECK: b[[BLOCK0:.*]] { type = CL }
// This call does not yield a result, because __qoala_wrapper0 request uses qubitID 0
// to create the entangled pair
// CHECK-NEXT: run_request() : __qoala_wrapper0
// CHECK: b[[BLOCK1:.*]] { type = CL }
// This call does not yield a result, because __qoala_wrapper1 request uses qubitID 1
// to create the entangled pair
// CHECK-NEXT: tuple<[[MEAS_0:.*]]; [[MEAS_1:.*]]; [[MEAS_2:.*]]> = run_subroutine() : __qoala_wrapper1

//CHECK: SUBROUTINE __qoala_wrapper1
// CHECK-NEXT: params: {{[[:space:]]}}
// CHECK-SAME: returns: m0, m1, m2
// CHECK-NEXT: uses: 0, 1, 2
// CHECK-NEXT: keeps: {{[[:space:]]}}
// CHECK-SAME: NETQASM_START
// CHECK-NEXT: set [[Q_REG0:.*]] 0
// CHECK-NEXT: set [[Q_REG1:.*]] 1
// CHECK-NEXT: set [[Q_REG2:.*]] 2
// CHECK-NEXT: meas [[Q_REG0]] [[M_REG0:.*]]
// CHECK-NEXT: meas [[Q_REG1]] [[M_REG1:.*]]
// CHECK-NEXT: meas [[Q_REG2]] [[M_REG2:.*]]
// CHECK-NEXT: store [[M_REG0]] @output[0]
// CHECK-NEXT: store [[M_REG1]] @output[1]
// CHECK-NEXT: store [[M_REG2]] @output[2]
// CHECK-NEXT: NETQASM_END

//CHECK: REQUEST __qoala_wrapper0
// CHECK-NEXT: callback_type: sequential
// CHECK-NEXT: callback: {{[[:space:]]}}
// CHECK-SAME: return_vars: {{[[:space:]]}}
// CHECK-SAME: remote_id: {Bob_id}
// CHECK-NEXT: epr_socket_id: 0
// CHECK-NEXT: num_pairs: 3
// CHECK-NEXT: virt_ids: increment 0
// CHECK-NEXT: timeout: 1000
// CHECK-NEXT: fidelity: 1.000000e+00
// CHECK-NEXT: type: create_keep
// CHECK-NEXT: role: create

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.request_routine @__qoala_wrapper0() -> (i32, i32, i32) {
    %0 = netqasm.qalloc  : i32
    %1 = netqasm.qalloc  : i32
    %2 = netqasm.qalloc  : i32
    netqasm.eprs %0  {remote = @Bob}
    netqasm.eprs %1  {remote = @Bob}
    netqasm.eprs %2  {remote = @Bob}
    netqasm.return %0, %1, %2 : i32, i32, i32
  }
  netqasm.local_routine @__qoala_wrapper1(%qubit0: i32, %qubit1: i32, %qubit2: i32) -> (i1, i1, i1) {
    %0 = netqasm.measure %qubit0  : i1
    %1 = netqasm.measure %qubit1  : i1
    %2 = netqasm.measure %qubit2  : i1
    netqasm.return %0, %1, %2 : i1, i1, i1
  }
  qoalahost.main_func @test_call_request_routines() {
    // Note: there is an implicit "^bb0" not-rendered block declaration here
    // so this "call" operation is the one and only operation of the
    // first block of the main function
    %0, %1, %2 = qoalahost.call @__qoala_wrapper0() : () -> (i32, i32, i32)
    ^bb1:
      %m0, %m1, %m2 = qoalahost.call @__qoala_wrapper1(%0, %1, %2) : (i32, i32, i32) -> (i1, i1, i1)
    ^bb2:
      qoalahost.return
  }
}
