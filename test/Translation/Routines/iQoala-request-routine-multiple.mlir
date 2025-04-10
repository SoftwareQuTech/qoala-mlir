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
// CHECK-NEXT: run_request() : __qoala_wrapper1
// CHECK: b[[BLOCK2:.*]] { type = CL }
// This call does not yield a result, because __qoala_wrapper2 request uses qubitID 2
// to create the entangled pair
// This call does not yield a result, because __qoala_wrapper0 request uses qubitID 0
// to create the entangled pair
// CHECK-NEXT: run_request() : __qoala_wrapper2
// CHECK: b[[BLOCK3:.*]] { type = CL }

//CHECK: REQUEST __qoala_wrapper0
// CHECK-NEXT: callback_type: sequential
// CHECK-NEXT: callback: {{[[:space:]]}}
// CHECK-SAME: return_vars: m0
// CHECK-NEXT: remote_id: {Bob_id}
// CHECK-NEXT: epr_socket_id: 0
// CHECK-NEXT: num_pairs: 1
// CHECK-NEXT: virt_ids: all 0
// CHECK-NEXT: timeout: 1000
// CHECK-NEXT: fidelity: 1.000000e+00
// CHECK-NEXT: type: create_keep
// CHECK-NEXT: role: create

//CHECK: REQUEST __qoala_wrapper1
// CHECK-NEXT: callback_type: sequential
// CHECK-NEXT: callback: {{[[:space:]]}}
// CHECK-SAME: return_vars: m0
// CHECK-NEXT: remote_id: {Bob_id}
// CHECK-NEXT: epr_socket_id: 0
// CHECK-NEXT: num_pairs: 1
// CHECK-NEXT: virt_ids: all 1
// CHECK-NEXT: timeout: 1000
// CHECK-NEXT: fidelity: 1.000000e+00
// CHECK-NEXT: type: create_keep
// CHECK-NEXT: role: create

//CHECK: REQUEST __qoala_wrapper2
// CHECK-NEXT: callback_type: sequential
// CHECK-NEXT: callback: {{[[:space:]]}}
// CHECK-SAME: return_vars: m0
// CHECK-NEXT: remote_id: {Bob_id}
// CHECK-NEXT: epr_socket_id: 0
// CHECK-NEXT: num_pairs: 1
// CHECK-NEXT: virt_ids: all 2
// CHECK-NEXT: timeout: 1000
// CHECK-NEXT: fidelity: 1.000000e+00
// CHECK-NEXT: type: create_keep
// CHECK-NEXT: role: create

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.request_routine @__qoala_wrapper0() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.eprs %0  {remote = @Bob}
    netqasm.return %0 : i32
  }
  netqasm.request_routine @__qoala_wrapper1() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.eprs %0  {remote = @Bob}
    netqasm.return %0 : i32
  }
  netqasm.request_routine @__qoala_wrapper2() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.eprs %0  {remote = @Bob}
    netqasm.return %0 : i32
  }
  qoalahost.main_func @test_call_request_routines() {
    // Note: there is an implicit "^bb0" not-rendered block declaration here
    // so this "call" operation is the one and only operation of the
    // first block of the main function
    %0 = qoalahost.call @__qoala_wrapper0() : () -> i32
    ^bb1:
      %1 = qoalahost.call @__qoala_wrapper1() : () -> i32
    ^bb2:
      %2 = qoalahost.call @__qoala_wrapper2() : () -> i32
    ^bb3:
      qoalahost.return
  }
}
