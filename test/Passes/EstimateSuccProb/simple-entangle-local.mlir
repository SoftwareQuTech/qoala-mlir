// RUN: qoala-opt %s --qoalahost-show-analysis-esp | FileCheck %s
// CHECK: [ESP]: 4.841724e-01

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @local_qubit() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.hadamard %0
    netqasm.return %0 : i32
  }
  netqasm.request_routine @entanglement() -> i32 {
    %1 = netqasm.qalloc  : i32
    netqasm.eprs %1  {remote = @Bob}
    netqasm.return %1 : i32
  }
  netqasm.local_routine @cnot(%0: i32, %1: i32) {
    netqasm.cnot %0, %1
    netqasm.return
  }
  netqasm.local_routine @meas_local(%0: i32) -> i1 {
    %m = netqasm.measure %0 : i1
    netqasm.return %m : i1
  }
  netqasm.local_routine @meas_ent(%0: i32) -> i1 {
    %m = netqasm.measure %0 : i1
    netqasm.return %m : i1
  }
  qoalahost.main_func @simple_entangle_local() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.call @entanglement() : () -> i32
    ^bb1:
        qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
        %1 = qoalahost.call @local_qubit() : () -> i32
    ^bb2:
        qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = ["block_0", "block_1"], predecessors = [], prev_comm = "", prev_ent = ""}
        qoalahost.call @cnot(%0, %1) : (i32, i32) -> ()
    ^bb3:
        qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = ["block_2"], predecessors = [], prev_comm = "", prev_ent = ""}
        %m0 = qoalahost.call @meas_ent(%0) : (i32) -> i1
    ^bb4:
        qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = ["block_2"], predecessors = [], prev_comm = "", prev_ent = ""}
        %m1 = qoalahost.call @meas_local(%1) : (i32) -> i1
  }
}
