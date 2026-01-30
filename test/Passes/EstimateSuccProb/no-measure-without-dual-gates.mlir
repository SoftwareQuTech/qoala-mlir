// RUN: qoala-opt %s --qoalahost-show-analysis-esp | FileCheck %s
// CHECK:  [ESP]: {{1\.0+e\+00}}

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
  netqasm.local_routine @single_gates(%arg0: i32, %arg1: i32, %zero_i32: i32, %one_i32: i32) {
    netqasm.rot_x %arg0, %zero_i32, %one_i32
    netqasm.rot_y %arg1, %zero_i32, %one_i32
    netqasm.rot_z %arg0, %zero_i32, %one_i32
    netqasm.hadamard %arg1
    netqasm.rot_y %arg0, %zero_i32, %one_i32
    netqasm.return
  }
  qoalahost.main_func @no_measure_without_dual_gates_esp() {
    qoalahost.blk_meta  {block_id = "block_10", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.remote_id_ref  {classical = true, quantum = false, remote = @Bob}
    %zero_i32 = arith.constant 0 : i32
    %one_i32 = arith.constant 1 : i32
    qoalahost.nop_term
  ^bb0:
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.call @entanglement() : () -> i32
  ^bb1:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %1 = qoalahost.call @local_qubit() : () -> i32
  ^bb2:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = ["block_0", "block_1", "block_10"], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.call @single_gates(%0, %1, %zero_i32, %one_i32) : (i32, i32, i32, i32) -> ()
  }
}
