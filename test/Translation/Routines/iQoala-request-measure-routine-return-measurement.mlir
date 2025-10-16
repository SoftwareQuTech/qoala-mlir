// UNSUPPORTED: true
// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s
// CHECK: META_START
// CHECK-NEXT: name: test_call_request_routine
// CHECK-NEXT: parameters: Bob_id
// CHECK-NEXT: csockets: 0 -> Bob
// CHECK-NEXT: epr_sockets: 0 -> Bob
// CHECK-NEXT: META_END
// CHECK: ^b[[BLOCK0:.*]] { type = QC; predecessors = []; dependencies = []; prev_comm = ; prev_ent = }
// CHECK-NEXT: %[[HOST_REG0:.*]] = run_request() : __qoala_wrapper0

//CHECK: REQUEST __qoala_wrapper0
// CHECK-NEXT: callback_type: WAIT_ALL
// CHECK-NEXT: callback: {{[[:space:]]}}
// CHECK-SAME: return_vars: m0
// CHECK-NEXT: remote_id: {Bob_id}
// CHECK-NEXT: epr_socket_id: 0
// CHECK-NEXT: num_pairs: 1
// CHECK-NEXT: virt_ids: all 0
// CHECK-NEXT: timeout: 1000
// CHECK-NEXT: fidelity: 1.000000e+00
// CHECK-NEXT: typ: measure_directly
// CHECK-NEXT: role: create

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32)
  netqasm.request_routine @__qoala_wrapper0() -> (i1, i1) {
    %0 = netqasm.qalloc  : i32
    %2 = netqasm.eprs_measure %0  {remote = @Bob} : i1
    netqasm.return %2: i1
  }
  qoalahost.main_func @test_call_request_routine() {
    // Note: there is an implicit "^bb0" not-rendered block declaration here
    // so this "call" operation is the one and only operation of the
    // first block of the main function
    qoalahost.blk_meta  {block_id = "block_0", dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0, %1 = qoalahost.call @__qoala_wrapper0() : () -> i1
    ^bb1:
      qoalahost.blk_meta  {block_id = "block_1", dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
      qoalahost.return
  }
}
