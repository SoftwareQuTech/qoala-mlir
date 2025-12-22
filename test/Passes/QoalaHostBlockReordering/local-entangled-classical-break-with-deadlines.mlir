// RUN: qoala-opt %s --qoalahost-reorder-blocks=with-deadlines | FileCheck %s

// XFAIL: true
// This test fails and it is expected to be fixed in #121. Check the big TODO note in BlockOrdering.cpp::runOnOperation

// "block_99"  is the first block with the remote reference id placeholder
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_99:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_1:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_0:.*]]", deadlines = {block_1 = 92 : i64}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_2:.*]]", deadlines = {block_1 = 198 : i64}, dependencies = ["[[BLOCK_0]]"], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_3:.*]]", deadlines = {block_1 = 302 : i64}, dependencies = ["[[BLOCK_1]]", "[[BLOCK_2]]"], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_5:.*]]", deadlines = {block_1 = 446 : i64}, dependencies = ["[[BLOCK_3]]"], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_4:.*]]", deadlines = {block_1 = 551 : i64}, dependencies = ["[[BLOCK_3]]"], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_8:.*]]", deadlines = {block_1 = 656 : i64}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = "[[BLOCK_1]]"}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_6:.*]]", deadlines = {block_1 = 1752 : i64}, dependencies = ["[[BLOCK_4]]"], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_7:.*]]", deadlines = {block_1 = 1845 : i64}, dependencies = ["[[BLOCK_5]]"], predecessors = [], prev_comm = "[[BLOCK_6]]", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_9:.*]]", deadlines = {block_1 = 1938 : i64}, dependencies = ["[[BLOCK_8]]"], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_10:.*]]", deadlines = {block_1 = 2042 : i64}, dependencies = ["[[BLOCK_9]]"], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_11:.*]]", deadlines = {block_1 = 2146 : i64}, dependencies = ["[[BLOCK_10]]"], predecessors = [], prev_comm = "", prev_ent = ""}


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
  netqasm.request_routine @entanglement2() -> i32 {
    %1 = netqasm.qalloc  : i32
    netqasm.eprs %1  {remote = @Bob}
    netqasm.return %1 : i32
  }
  netqasm.local_routine @corr_x(%0: i32) {
    netqasm.rot_x %0 (314 : ui32, 100 : ui32)
    netqasm.return
  }
  netqasm.local_routine @corr_z(%0: i32) {
    netqasm.rot_z %0 (314 : ui32, 100 : ui32)
    netqasm.return
  }
  netqasm.local_routine @meas(%1: i32) -> i1 {
    %m1 = netqasm.measure %1 : i1
    netqasm.return %m1 : i1
  }
  qoalahost.main_func @test_reordering_teleport() {
    qoalahost.blk_meta  {block_id = "block_99", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.remote_id_ref  {classical = true, quantum = true, remote = @Bob}
    qoalahost.nop_term
    ^bb1:
        qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
        %0 = qoalahost.call @local_qubit() : () -> i32
    ^bb2:
        qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
        %1 = qoalahost.call @entanglement() : () -> i32
    ^bb3:
        qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = ["block_0"], predecessors = [], prev_comm = "", prev_ent = ""}
        qoalahost.call @bsm_h(%0) : (i32) -> ()
    ^bb4:
        qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = ["block_1", "block_2"], predecessors = [], prev_comm = "", prev_ent = ""}
        qoalahost.call @bsm_cnot(%0, %1) : (i32, i32) -> ()
    ^bb5:
        qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = ["block_3"], predecessors = [], prev_comm = "", prev_ent = ""}
        %m0 = qoalahost.call @bsm_meas_local(%0) : (i32) -> i1
    ^bb6:
        qoalahost.blk_meta  {block_id = "block_5", deadlines = {}, dependencies = ["block_3"], predecessors = [], prev_comm = "", prev_ent = ""}
        %m1 = qoalahost.call @bsm_meas_ent(%1) : (i32) -> i1
    ^bb7:
        qoalahost.blk_meta  {block_id = "block_6", deadlines = {}, dependencies = ["block_4"], predecessors = [], prev_comm = "", prev_ent = ""}
        %m0_ext = arith.extsi %m0 : i1 to i32
        %m0_tensor = tensor.from_elements %m0_ext : tensor<1xi32>
        qoalahost.send_ints %m0_tensor {remote = @Bob} : tensor<1xi32>
        qoalahost.nop_term
    ^bb8:
        qoalahost.blk_meta  {block_id = "block_7", deadlines = {}, dependencies = ["block_5"], predecessors = [], prev_comm = "block_6", prev_ent = ""}
        %m1_ext = arith.extsi %m1 : i1 to i32
        %m1_tensor = tensor.from_elements %m1_ext : tensor<1xi32>
        qoalahost.send_ints %m1_tensor {remote = @Bob} : tensor<1xi32>
        qoalahost.nop_term
    ^bb9:
        qoalahost.blk_meta  {block_id = "block_8", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = "block_1"}
        %2 = qoalahost.call @entanglement2() : () -> i32
    ^bb10:
        qoalahost.blk_meta  {block_id = "block_9", deadlines = {}, dependencies = ["block_8"], predecessors = [], prev_comm = "", prev_ent = ""}
        qoalahost.call @corr_x(%2) : (i32) -> ()
    ^bb11:
        qoalahost.blk_meta  {block_id = "block_10", deadlines = {}, dependencies = ["block_9"], predecessors = [], prev_comm = "", prev_ent = ""}
        qoalahost.call @corr_z(%2) : (i32) -> ()
    ^bb12:
        qoalahost.blk_meta  {block_id = "block_11", deadlines = {}, dependencies = ["block_10"], predecessors = [], prev_comm = "", prev_ent = ""}
        %m2 = qoalahost.call @meas(%2) : (i32) -> i1
  }
}
