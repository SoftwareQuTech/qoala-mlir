// RUN: qoala-opt %s --qoalahost-show-analysis-esp | FileCheck %s
//CHECK:  [ESP]: 5.295039e-01

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @local_qubit() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.return %0 : i32
  }
  netqasm.request_routine @entanglement() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.eprs %0 {remote = @Bob}
    netqasm.return %0 : i32
  }
  netqasm.local_routine @bsm_cnot(%arg0: i32, %arg1: i32) {
    netqasm.cnot %arg0, %arg1
    netqasm.return
  }
  netqasm.local_routine @bsm_h(%arg0: i32) {
    netqasm.hadamard %arg0
    netqasm.return
  }
  netqasm.local_routine @bsm_meas_local(%arg0: i32) -> i1 {
    %0 = netqasm.measure %arg0 : i1
    netqasm.return %0 : i1
  }
  netqasm.local_routine @bsm_meas_ent(%arg0: i32) -> i1 {
    %0 = netqasm.measure %arg0 : i1
    netqasm.return %0 : i1
  }
  qoalahost.main_func @test_teleport_esp() {
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.call @entanglement() : () -> i32
  ^bb1:
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %1 = qoalahost.call @local_qubit() : () -> i32
  ^bb2:
    qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = ["block_0"], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.call @bsm_h(%1) : (i32) -> ()
  ^bb3:
    qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = ["block_1", "block_2"], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.call @bsm_cnot(%1, %0) : (i32, i32) -> ()
  ^bb4:
    qoalahost.blk_meta  {block_id = "block_5", deadlines = {}, dependencies = ["block_3"], predecessors = [], prev_comm = "", prev_ent = ""}
    %2 = qoalahost.call @bsm_meas_ent(%0) : (i32) -> i1
  ^bb5:
    qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = ["block_3"], predecessors = [], prev_comm = "", prev_ent = ""}
    %3 = qoalahost.call @bsm_meas_local(%1) : (i32) -> i1
  ^bb6:
    qoalahost.blk_meta  {block_id = "block_6", deadlines = {}, dependencies = ["block_4"], predecessors = [], prev_comm = "", prev_ent = ""}
    %4 = arith.extsi %3 : i1 to i32
    %from_elements = tensor.from_elements %4 : tensor<1xi32>
    qoalahost.send_ints %from_elements {remote = @Bob} : tensor<1xi32>
    qoalahost.nop_term
  ^bb7:
    qoalahost.blk_meta  {block_id = "block_7", deadlines = {}, dependencies = ["block_5"], predecessors = [], prev_comm = "block_6", prev_ent = ""}
    %5 = arith.extsi %2 : i1 to i32
    %from_elements_0 = tensor.from_elements %5 : tensor<1xi32>
    qoalahost.send_ints %from_elements_0 {remote = @Bob} : tensor<1xi32>
    qoalahost.nop_term
  }
}
