// RUN: qoala-opt %s --qoalahost-reorder-blocks | FileCheck %s

// CHECK: qoalahost.blk_meta  {block_id = "block_1", dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "block_0", dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "block_2", dependencies = ["block_0"], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "block_3", dependencies = ["block_1", "block_2"], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "block_5", dependencies = ["block_3"], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "block_4", dependencies = ["block_3"], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "block_6", dependencies = ["block_4", "block_5"], predecessors = [], prev_comm = "", prev_ent = ""}

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @local_qubit() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.return %0 : i32
  }
  netqasm.request_routine @entanglement() -> i32 {
    %1 = netqasm.qalloc  : i32
    netqasm.eprs %1  {remote = @Bob}
    netqasm.return %1 : i32
  }
  netqasm.local_routine @bsm_cnot(%0: i32, %1: i32) {
    netqasm.cnot %0, %1
    netqasm.return
  }
  netqasm.local_routine @bsm_h(%0: i32) {
    netqasm.hadamard %0
    netqasm.return
  }
  netqasm.local_routine @bsm_meas_local(%0: i32) -> i1 {
    %m0 = netqasm.measure %0 : i1
    netqasm.return %m0 : i1
  }
  netqasm.local_routine @bsm_meas_ent(%1: i32) -> i1 {
    %m1 = netqasm.measure %1 : i1
    netqasm.return %m1 : i1
  }
  qoalahost.main_func @test_reordering_teleport() {
    qoalahost.blk_meta  {block_id = "block_0", dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.call @local_qubit() : () -> i32
    ^bb1:
        qoalahost.blk_meta  {block_id = "block_1", dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
        %1 = qoalahost.call @entanglement() : () -> i32
    ^bb2:
        qoalahost.blk_meta  {block_id = "block_2", dependencies = ["block_0"], predecessors = [], prev_comm = "", prev_ent = ""}
        qoalahost.call @bsm_h(%0) : (i32) -> ()
    ^bb3:
        qoalahost.blk_meta  {block_id = "block_3", dependencies = ["block_1", "block_2"], predecessors = [], prev_comm = "", prev_ent = ""}
        qoalahost.call @bsm_cnot(%0, %1) : (i32, i32) -> ()
    ^bb4:
        qoalahost.blk_meta  {block_id = "block_4", dependencies = ["block_3"], predecessors = [], prev_comm = "", prev_ent = ""}
        %m0 = qoalahost.call @bsm_meas_local(%0) : (i32) -> i1
    ^bb5:
        qoalahost.blk_meta  {block_id = "block_5", dependencies = ["block_3"], predecessors = [], prev_comm = "", prev_ent = ""}
        %m1 = qoalahost.call @bsm_meas_ent(%1) : (i32) -> i1
    ^bb6:
        qoalahost.blk_meta  {block_id = "block_6", dependencies = ["block_4", "block_5"], predecessors = [], prev_comm = "", prev_ent = ""}
        qoalahost.return %m0, %m1 : i1, i1
  }
}
