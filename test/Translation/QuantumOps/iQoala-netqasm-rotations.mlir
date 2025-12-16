// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s
// CHECK: META_START
// CHECK-NEXT: name: test_local_routine_rotations
// CHECK-NEXT: parameters:
// CHECK-NEXT: csockets:
// CHECK-NEXT: epr_sockets:
// CHECK-NEXT: META_END
// CHECK: ^b[[BLOCK0:.*]] { type = QL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: %[[RES_0:.*]] = run_subroutine() : __qoala_wrapper0

//CHECK: SUBROUTINE __qoala_wrapper0
// CHECK-NEXT: params:
// CHECK-NEXT: returns:
// CHECK-NEXT: uses:
// CHECK-NEXT: keeps:
// CHECK-NEXT: request:
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: set Q[[QBIT0:.*]] 0
// CHECK-NEXT: init Q[[QBIT0]]
// CHECK-NEXT: rot_x Q[[QBIT0]] 0 0
// CHECK-NEXT: rot_y Q[[QBIT0]] 1 0
// CHECK-NEXT: rot_z Q[[QBIT0]] 1 1
// CHECK-NEXT: meas Q[[QBIT0]] M[[M_REG0:.*]]
// CHECK-NEXT: store M[[M_REG0]] @output[0]
// CHECK-NEXT: NETQASM_END

module {
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @__qoala_wrapper0() -> i1 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.rot_x %0 (0 : ui32, 0 : ui32)
    netqasm.rot_y %0 (1 : ui32, 0 : ui32)
    netqasm.rot_z %0 (1 : ui32, 1 : ui32)
    %1 = netqasm.measure %0 : i1
    netqasm.return %1 : i1
  }
  qoalahost.main_func @test_local_routine_rotations() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.call @__qoala_wrapper0() : () -> i1
  ^bb1:
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.return
  }
}