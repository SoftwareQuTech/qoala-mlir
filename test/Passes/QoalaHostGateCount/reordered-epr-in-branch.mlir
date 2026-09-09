// Regression test: a branch destination whose blk_meta dependency -- the block
// initializing the qubit it operates on -- has not been visited yet.
//
// The block reorderer may hoist an EPR request past a conditional branch, so
// the destination of that branch depends on a block that comes later in
// physical order. Entering the destination straight away meant the analysis
// saw a gate on a qubit it had never seen initialized, and looking that qubit
// up asserted. The traversal now drains ready blocks until the destination's
// dependencies are met, so block_4 is counted before block_5 uses its qubit.
//
// Both arms apply one one-qubit gate to the same qubit and the join block
// measures it, so the reported count is 1 either way; the point of the test is
// that the analysis completes at all.

// RUN: qoala-opt %s --qoalahost-show-analysis-gate-count | FileCheck %s
// CHECK:  [Gate Count]:
// CHECK:  - One-qubit gates: 1
// CHECK:  - Two-qubit gates: 0
// CHECK:  - Total gates: 1
// CHECK:  Detailed gate count:
// CHECK:  - One-qubit gates:
// CHECK:    * qubit[block_4::2]: 1
// CHECK:  - Two-qubit gates:
// CHECK:    * qubit[block_4::2]: 0
// CHECK:  -  All gates:
// CHECK:    * qubit[block_4::2]: 1

module {
  qremote.remote @Alice
  netqasm.request_routine @__qoala_wrapper0() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.eprs %0 {remote = @Alice}
    netqasm.return %0 : i32
  }
  netqasm.local_routine @__qoala_wrapper1(%arg0: i32) {
    netqasm.x %arg0
    netqasm.return
  }
  netqasm.local_routine @__qoala_wrapper2(%arg0: i32) {
    netqasm.z %arg0
    netqasm.return
  }
  netqasm.local_routine @__qoala_wrapper3(%arg0: i32) -> i1 {
    %0 = netqasm.measure %arg0 : i1
    netqasm.return %0 : i1
  }
  qoalahost.main_func @reordered_epr_in_branch() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.remote_id_ref  {classical = true, quantum = true, remote = @Alice}
    qoalahost.nop_term
  ^bb1:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %c1_i32 = arith.constant 1 : i32
    qoalahost.nop_term
  ^bb2:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = ["block_0"], predecessors = [], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.recv_int  {remote = @Alice} : i32
  ^bb3:  // no predecessors
    // Branches before the qubit exists. Physically ahead of block_4, so the
    // traversal reaches this conditional first.
    qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = ["block_1", "block_2"], predecessors = [], prev_comm = "", prev_ent = ""}
    %1 = arith.cmpi eq, %0, %c1_i32 : i32
    cf.cond_br %1, ^bb5, ^bb7
  ^bb4:  // no predecessors
    // The hoisted EPR request: initializes the qubit both arms use, but sits
    // after the branch and is not a destination of it.
    qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = ["block_0"], predecessors = [], prev_comm = "", prev_ent = ""}
    %2 = qoalahost.call @__qoala_wrapper0() : () -> i32
  ^bb5:  // pred: ^bb3
    qoalahost.blk_meta  {block_id = "block_5", deadlines = {}, dependencies = ["block_4"], predecessors = ["block_3"], prev_comm = "", prev_ent = ""}
    qoalahost.call @__qoala_wrapper1(%2) : (i32) -> ()
  ^bb6:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_6", deadlines = {}, dependencies = ["block_5"], predecessors = ["block_5"], prev_comm = "", prev_ent = ""}
    cf.br ^bb9
  ^bb7:  // pred: ^bb3
    qoalahost.blk_meta  {block_id = "block_7", deadlines = {}, dependencies = ["block_4"], predecessors = ["block_3"], prev_comm = "", prev_ent = ""}
    qoalahost.call @__qoala_wrapper2(%2) : (i32) -> ()
  ^bb8:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_8", deadlines = {}, dependencies = ["block_7"], predecessors = ["block_7"], prev_comm = "", prev_ent = ""}
    cf.br ^bb9
  ^bb9:  // 2 preds: ^bb6, ^bb8
    // Measuring flushes the buffered one-qubit gate into the reported count.
    qoalahost.blk_meta  {block_id = "block_9", deadlines = {}, dependencies = ["block_4"], predecessors = ["block_6", "block_8"], prev_comm = "", prev_ent = ""}
    %3 = qoalahost.call @__qoala_wrapper3(%2) : (i32) -> i1
  ^bb10:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_10", deadlines = {}, dependencies = ["block_9"], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.return %3 : i1
  }
}
