// RUN: qoala-opt %s --qoalahost-show-analysis-esp | FileCheck %s
//CHECK:  [ESP]: 5.245710e-01

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
  netqasm.local_routine @cnot_rot(%arg0: i32, %arg1: i32) {
    netqasm.cnot %arg0, %arg1
    netqasm.rot_x %arg0 (0 : ui32, 1 : ui32)
    netqasm.rot_y %arg0 (0 : ui32, 1 : ui32)
    netqasm.rot_z %arg0 (0 : ui32, 1 : ui32)
    netqasm.hadamard %arg0
    netqasm.return
  }
  netqasm.local_routine @meas_local(%arg0: i32) -> i1 {
    %0 = netqasm.measure %arg0 : i1
    netqasm.return %0 : i1
  }
  qoalahost.main_func @no_measure_esp() {
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.call @entanglement() : () -> i32
  ^bb1:
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %1 = qoalahost.call @local_qubit() : () -> i32
  ^bb2:
    qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = ["block_0", "block_1"], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.call @cnot_rot(%0, %1) : (i32, i32) -> ()
  ^bb3:
    qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = ["block_2"], predecessors = [], prev_comm = "", prev_ent = ""}
    %2 = qoalahost.call @meas_local(%1) : (i32) -> i1
  }
}
