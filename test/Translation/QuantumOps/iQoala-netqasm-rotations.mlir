// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s
// CHECK: META START
// CHECK-NEXT: name: test_local_routine_ret_one_val
// CHECK-NEXT: parameters: Bob
// CHECK-NEXT: csockets: 0 -> Bob
// CHECK-NEXT: epr_sockets: 0 -> Bob
// CHECK-NEXT: META END
// CHECK-NEXT: b[[BLOCK0:.*]] { type = CL }
// CHECK-NEXT: %[[HOST_REG0:.*]] = assign_cval () : 3

//CHECK: SUBROUTINE __qoala_wrapper0
// CHECK-NEXT: params:
// CHECK-NEXT: returns:
// CHECK-NEXT: uses:
// CHECK-NEXT: keeps:
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: set Q[[Q_REG0:.*]] 0
// init Q[[Q_REG0:.*]]
// CHECK-NEXT: rot_x %[[QBIT0]] 0 0
// CHECK-NEXT: rot_y %[[QBIT0]] 1 0
// CHECK-NEXT: rot_z %[[QBIT0]] 1 1
// meas %[[QBIT0]] M[[M_REG0:.*]]
// store M[[M_REG0]] @output[0]
// CHECK-NEXT: NETQASM_END

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @__qoala_wrapper0() -> i1 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.rot_x %0 (0 : ui32, 0 : ui32)
    netqasm.rot_y %0 (0 : ui32, 0 : ui32)
    netqasm.rot_z %0 (0 : ui32, 0 : ui32)
    %1 = netqasm.measure %0 : i1
    netqasm.return %1 : i1
  }
  qoalahost.main_func @test_local_routine_ret_one_val() {
    %0 = qoalahost.call @__qoala_wrapper0() : () -> i1
  ^bb1:
    qoalahost.return
  }
}