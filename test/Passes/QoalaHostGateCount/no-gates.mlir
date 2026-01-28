// RUN: qoala-opt %s --qoalahost-show-analysis-gate-count | FileCheck %s
// CHECK:  [Gate Count]:
// CHECK:  - One-qubit gates: 0
// CHECK:  - Two-qubit gates: 0
// CHECK:  - Total gates: 0
// CHECK:  Detailed gate count:
// CHECK:  - One-qubit gates:
// CHECK:    * qubit[block_0::2]: 0
// CHECK:    * qubit[block_1::2]: 0
// CHECK:  - Two-qubit gates:
// CHECK:    * qubit[block_0::2]: 0
// CHECK:    * qubit[block_1::2]: 0
// CHECK:  -  All gates:
// CHECK:    * qubit[block_0::2]: 0
// CHECK:    * qubit[block_1::2]: 0

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @local_qubit() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.return %0 : i32
  }
  netqasm.request_routine @entangle_qubit() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.eprs %0 {remote = @Bob}
    netqasm.return %0 : i32
  }
  netqasm.local_routine @meas_local(%arg0: i32) -> i1 {
    %0 = netqasm.measure %arg0 : i1
    netqasm.return %0 : i1
  }
  netqasm.local_routine @meas_ent(%arg0: i32) -> i1 {
    %0 = netqasm.measure %arg0 : i1
    netqasm.return %0 : i1
  }
  qoalahost.main_func @test_no_gates_count() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.call @local_qubit() : () -> i32
  ^bb1:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %1 = qoalahost.call @entangle_qubit() : () -> i32
  ^bb2:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = ["block_0"], predecessors = [], prev_comm = "", prev_ent = ""}
    %2 = qoalahost.call @meas_local(%0) : (i32) -> i1
  ^bb3:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = ["block_1"], predecessors = [], prev_comm = "", prev_ent = ""}
    %3 = qoalahost.call @meas_ent(%1) : (i32) -> i1
  ^bb4:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.return
  }
}