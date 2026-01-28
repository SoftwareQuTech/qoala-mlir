// RUN: qoala-opt %s --qoalahost-show-analysis-gate-count | FileCheck %s
// CHECK:  [Gate Count]:
// CHECK:  - One-qubit gates: 1
// CHECK:  - Two-qubit gates: 2
// CHECK:  - Total gates: 3
// CHECK:  Detailed gate count:
// CHECK:  - One-qubit gates:
// CHECK:    * qubit[block_0::2]: 0
// CHECK:    * qubit[block_1::2]: 1
// CHECK:    * qubit[block_4::2]: 0
// CHECK:  - Two-qubit gates:
// CHECK:    * qubit[block_0::2]: 1
// CHECK:    * qubit[block_1::2]: 2
// CHECK:    * qubit[block_4::2]: 1
// CHECK: -  All gates:
// CHECK:    * qubit[block_0::2]: 1
// CHECK:    * qubit[block_1::2]: 3
// CHECK:    * qubit[block_4::2]: 1

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @local_qubit0() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.return %0 : i32
  }
  netqasm.local_routine @local_qubit1() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.return %0 : i32
  }
  netqasm.local_routine @cnot0(%arg0: i32, %arg1: i32) {
    netqasm.cnot %arg0, %arg1
    netqasm.return
  }
  netqasm.local_routine @rot(%arg0: i32) {
    netqasm.rot_x %arg0 (1 : ui32, 0 : ui32)
    netqasm.return
  }
  netqasm.request_routine @entangle_qubit() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.eprs %0 {remote = @Bob}
    netqasm.return %0 : i32
  }
  netqasm.local_routine @cnot1(%arg0: i32, %arg1: i32) {
    netqasm.cnot %arg0, %arg1
    netqasm.return
  }
  netqasm.local_routine @meas(%arg0: i32) -> i1 {
    %0 = netqasm.measure %arg0 : i1
    netqasm.return %0 : i1
  }
  qoalahost.main_func @test_simple_gate_count() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.call @local_qubit0() : () -> i32
  ^bb1:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %1 = qoalahost.call @local_qubit1() : () -> i32
  ^bb2:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = ["block_0", "block_1"], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.call @cnot0(%0, %1) : (i32, i32) -> ()
  ^bb3:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = ["block_2"], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.call @rot(%1) : (i32) -> ()
  ^bb4:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %2 = qoalahost.call @entangle_qubit() : () -> i32
  ^bb5:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_5", deadlines = {}, dependencies = ["block_3", "block_4"], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.call @cnot1(%2, %1) : (i32, i32) -> ()
  ^bb6:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_6", deadlines = {}, dependencies = ["block_5"], predecessors = [], prev_comm = "", prev_ent = ""}
    %3 = qoalahost.call @meas(%1) : (i32) -> i1
  ^bb7:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_7", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.return
  }
}