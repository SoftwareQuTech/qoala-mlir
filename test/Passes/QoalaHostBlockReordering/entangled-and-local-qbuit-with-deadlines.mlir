// RUN: qoala-opt %s --qoalahost-reorder-blocks=with-deadlines | FileCheck %s

// The checks for deadlines value are +/-1. depending on CPU performance, the computed value might slightly change. Howerver, since we are computing
// **soft** deadlines, htis is not an issue.
// CHECK: qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}

// 79 +/-1  -> 78|79|80
// CHECK: qoalahost.blk_meta  {block_id = "block_0", deadlines = {block_1 = {{(78|79|80)}} : i64}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
// 172 +/-1 -> 171|172|173
// CHECK: qoalahost.blk_meta  {block_id = "block_2", deadlines = {block_1 = {{(171|172|173)}} : i64}, dependencies = ["block_0"], predecessors = [], prev_comm = "", prev_ent = ""}
// 263 +/-1 -> 262|263|264
// CHECK: qoalahost.blk_meta  {block_id = "block_3", deadlines = {block_1 = {{(262|263|264)}} : i64}, dependencies = ["block_1", "block_2"], predecessors = [], prev_comm = "", prev_ent = ""}
// 394 +/-1 -> 393|394|395
// CHECK: qoalahost.blk_meta  {block_id = "block_4", deadlines = {block_1 = {{(393|394|395)}} : i64}, dependencies = ["block_3"], predecessors = [], prev_comm = "", prev_ent = ""}
// 486 +/-1 -> 485|486|487
// CHECK: qoalahost.blk_meta  {block_id = "block_5", deadlines = {block_1 = {{(485|486|487)}} : i64}, dependencies = ["block_3"], predecessors = [], prev_comm = "", prev_ent = ""}
// 578 +/-1 -> 577|578|579
// CHECK: qoalahost.blk_meta  {block_id = "block_6", deadlines = {block_1 = {{(577|578|579)}} : i64}, dependencies = ["block_4"], predecessors = [], prev_comm = "", prev_ent = ""}
// 658 +/-1 -> 657|658|659
// CHECK: qoalahost.blk_meta  {block_id = "block_7", deadlines = {block_1 = {{(657|658|659)}} : i64}, dependencies = ["block_5"], predecessors = [], prev_comm = "block_6", prev_ent = ""}

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
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.call @local_qubit() : () -> i32
    ^bb1:
        qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
        %1 = qoalahost.call @entanglement() : () -> i32
    ^bb2:
        qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = ["block_0"], predecessors = [], prev_comm = "", prev_ent = ""}
        qoalahost.call @bsm_h(%0) : (i32) -> ()
    ^bb3:
        qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = ["block_1", "block_2"], predecessors = [], prev_comm = "", prev_ent = ""}
        qoalahost.call @bsm_cnot(%0, %1) : (i32, i32) -> ()
    ^bb4:
        qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = ["block_3"], predecessors = [], prev_comm = "", prev_ent = ""}
        %m0 = qoalahost.call @bsm_meas_local(%0) : (i32) -> i1
    ^bb5:
        qoalahost.blk_meta  {block_id = "block_5", deadlines = {}, dependencies = ["block_3"], predecessors = [], prev_comm = "", prev_ent = ""}
        %m1 = qoalahost.call @bsm_meas_ent(%1) : (i32) -> i1
    ^bb6:
        qoalahost.blk_meta  {block_id = "block_6", deadlines = {}, dependencies = ["block_4"], predecessors = [], prev_comm = "", prev_ent = ""}
        %m0_ext = arith.extsi %m0 : i1 to i32
        %m0_tensor = tensor.from_elements %m0_ext : tensor<1xi32>
        qoalahost.send_ints %m0_tensor {remote = @Bob} : tensor<1xi32>
        qoalahost.nop_term
    ^bb7:
        qoalahost.blk_meta  {block_id = "block_7", deadlines = {}, dependencies = ["block_5"], predecessors = [], prev_comm = "block_6", prev_ent = ""}
        %m1_ext = arith.extsi %m1 : i1 to i32
        %m1_tensor = tensor.from_elements %m1_ext : tensor<1xi32>
        qoalahost.send_ints %m1_tensor {remote = @Bob} : tensor<1xi32>
        qoalahost.nop_term

  }
}
