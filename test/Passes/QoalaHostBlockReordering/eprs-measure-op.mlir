// RUN: qoala-opt %s --qoalahost-reorder-blocks | FileCheck %s

// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_0:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_1:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_3:.*]]", deadlines = {}, dependencies = ["[[BLOCK_1]]"], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_2:.*]]", deadlines = {}, dependencies = ["[[BLOCK_3]]"], predecessors = [], prev_comm = "", prev_ent = ""}

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @local_qubit() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.return %0 : i32
  }
  netqasm.request_routine @entanglement() -> i1 {
    %1 = netqasm.qalloc  : i32
    %i1 = netqasm.eprs_measure %1  {remote = @Bob} : i1
    netqasm.return %i1 : i1
  }
  netqasm.local_routine @h(%0: i32) {
    netqasm.hadamard %0
    netqasm.return
  }
  netqasm.local_routine @meas_local(%0: i32) -> i1 {
    %m0 = netqasm.measure %0 : i1
    netqasm.return %m0 : i1
  }
  qoalahost.main_func @test_reordering_teleport() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.remote_id_ref  {classical = true, quantum = true, remote = @Bob}
    qoalahost.nop_term
    ^bb1:
        qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
        %0 = qoalahost.call @local_qubit() : () -> i32
    ^bb2:
        qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
        %m1 = qoalahost.call @entanglement() : () -> i1
    ^bb3:
        qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = ["block_1"], predecessors = [], prev_comm = "", prev_ent = ""}
        qoalahost.call @h(%0) : (i32) -> ()
    ^bb4:
        qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = ["block_2"], predecessors = [], prev_comm = "", prev_ent = ""}
        %m0 = qoalahost.call @meas_local(%0) : (i32) -> i1
    ^bb5:
        qoalahost.blk_meta  {block_id = "block_5", deadlines = {}, dependencies = ["block_4"], predecessors = [], prev_comm = "", prev_ent = ""}
        %m0_ext = arith.extsi %m0 : i1 to i32
        %m0_tensor = tensor.from_elements %m0_ext : tensor<1xi32>
        qoalahost.send_ints %m0_tensor {remote = @Bob} : tensor<1xi32>
        qoalahost.nop_term
    ^bb6:
        qoalahost.blk_meta  {block_id = "block_6", deadlines = {}, dependencies = ["block_2"], predecessors = [], prev_comm = "block_5", prev_ent = ""}
        %m1_ext = arith.extsi %m1 : i1 to i32
        %m1_tensor = tensor.from_elements %m1_ext : tensor<1xi32>
        qoalahost.send_ints %m1_tensor {remote = @Bob} : tensor<1xi32>
        qoalahost.nop_term

  }
}
