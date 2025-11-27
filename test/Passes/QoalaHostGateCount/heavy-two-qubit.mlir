// RUN: qoala-opt %s --qoalahost-show-analysis-gate-count | FileCheck %s
// CHECK:  [Gate Count]:
// CHECK:  - One-qubit gates: 2
// CHECK:  - Two-qubit gates: 4
// CHECK:  - Total gates: 6
// CHECK:  Detailed gate count:
// CHECK:  - One-qubit gates:
// CHECK:    * qubit[block_0::2]: 2
// CHECK:    * qubit[block_1::2]: 0
// CHECK:  - Two-qubit gates:
// CHECK:    * qubit[block_0::2]: 4
// CHECK:    * qubit[block_1::2]: 4
// CHECK:  -  All gates:
// CHECK:    * qubit[block_0::2]: 6
// CHECK:    * qubit[block_1::2]: 4

module {
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @local_qubit0() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.hadamard %0
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
  netqasm.local_routine @cnot1(%arg0: i32, %arg1: i32) {
    netqasm.cnot %arg0, %arg1
    netqasm.return
  }
  netqasm.local_routine @hadamard(%arg0: i32) {
    netqasm.hadamard %arg0
    netqasm.return
  }
  netqasm.local_routine @cz(%arg0: i32, %arg1: i32) {
    netqasm.cz %arg0, %arg1
    netqasm.cz %arg0, %arg1
    netqasm.return
  }
  qoalahost.main_func @test_heavy_two_qubit_gate_count() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.call @local_qubit0() : () -> i32
  ^bb1:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %1 = qoalahost.call @local_qubit1() : () -> i32
  ^bb2:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = ["block_0", "block_1"], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.call @cnot0(%1, %0) : (i32, i32) -> ()
  ^bb3:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = ["block_2"], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.call @cnot1(%0, %1) : (i32, i32) -> ()
  ^bb4:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = ["block_3"], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.call @hadamard(%0) : (i32) -> ()
  ^bb5:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_5", deadlines = {}, dependencies = ["block_4"], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.call @cz(%0, %1) : (i32, i32) -> ()
  ^bb6:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_6", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.return
  }
}