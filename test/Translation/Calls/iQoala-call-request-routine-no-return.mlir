// RUN: qoala-translate %s --split-input-file --mlir-to-iqoala | FileCheck %s
// CHECK: META START
// CHECK-NEXT: name: test_call_request_routine
// CHECK-NEXT: parameters: Bob
// CHECK-NEXT: csockets: 0 -> Bob
// CHECK-NEXT: epr_sockets: 0 -> Bob
// CHECK-NEXT: META END
// CHECK-NEXT: b[[BLOCK0:.*]] { type = CL }
// CHECK-NEXT: b[[BLOCK1:.*]] { type = CL }

//CHECK: REQUEST __qoala_wrapper0
// CHECK-NEXT: callback_type: sequential
// CHECK-NEXT: callback:
// CHECK-NEXT: return_vars:
// CHECK-NEXT: remote_id: {}
// CHECK-NEXT: epr_socket_id: 0
// CHECK-NEXT: num_pairs: 0
// CHECK-NEXT: virt_ids: all 0
// CHECK-NEXT: timeout: 1000
// CHECK-NEXT: fidelity: 1.000000e+00
// CHECK-NEXT: type: create_keep
// CHECK-NEXT: role: create

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.request_routine @__qoala_wrapper0() -> () {
    %0 = netqasm.qalloc  : i32
    netqasm.eprs %0  {remote = @Bob}
    netqasm.return
  }
  qoalahost.main_func @test_call_request_routine() {
    // Note: there is an implicit "^bb0" not-rendered block declaration here
    // so this "call" operation is the one and only operation of the
    // first block of the main function
    qoalahost.call @__qoala_wrapper0() : () -> ()
    ^bb1:
        qoalahost.return
  }
}
