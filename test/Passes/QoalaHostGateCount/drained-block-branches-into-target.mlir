// Regression test: a block visited while draining branches straight into the
// block the drain was running for.
//
// block_1 jumps to block_5, whose dependency (block_4) is not met yet, so the
// traversal drains ready blocks. That drain reaches block_4, which ends in
// `cf.br ^bb5` -- so block_5 is visited and counted from inside the drain.
// Control then returns to the outer frame still holding block_5, which must
// notice it has already been counted. Without that check the block's
// operations are processed a second time and its cnot, which is committed
// immediately rather than buffered, is counted twice (Two-qubit gates: 2).

// RUN: qoala-opt %s --qoalahost-show-analysis-gate-count | FileCheck %s
// CHECK:  [Gate Count]:
// CHECK:  - One-qubit gates: 1
// CHECK:  - Two-qubit gates: 1
// CHECK:  - Total gates: 2
// CHECK:  Detailed gate count:
// CHECK:  - One-qubit gates:
// CHECK:    * qubit[block_2::2]: 1
// CHECK:    * qubit[block_3::2]: 0
// CHECK:  - Two-qubit gates:
// CHECK:    * qubit[block_2::2]: 1
// CHECK:    * qubit[block_3::2]: 1
// CHECK:  -  All gates:
// CHECK:    * qubit[block_2::2]: 2
// CHECK:    * qubit[block_3::2]: 1

module {
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
  qoalahost.main_func @drained_block_branches_into_target() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.nop_term
  ^bb1:  // no predecessors
    // Jumps ahead to block_5, whose dependency has not been visited yet.
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    cf.br ^bb5
  ^bb2:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.call @local_qubit0() : () -> i32
  ^bb3:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %1 = qoalahost.call @local_qubit1() : () -> i32
  ^bb4:  // no predecessors
    // Drained on block_5's behalf, then branches into block_5 itself.
    qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = ["block_2", "block_3"], predecessors = [], prev_comm = "", prev_ent = ""}
    cf.br ^bb5
  ^bb5:  // 2 preds: ^bb1, ^bb4
    qoalahost.blk_meta  {block_id = "block_5", deadlines = {}, dependencies = ["block_4"], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.call @cnot0(%0, %1) : (i32, i32) -> ()
  ^bb6:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_6", deadlines = {}, dependencies = ["block_5"], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.return
  }
}
