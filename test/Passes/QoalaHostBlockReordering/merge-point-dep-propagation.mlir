// RUN: qoala-opt %s --qoalahost-reorder-blocks | FileCheck %s
//
// Regression test: at a merge point with both dependencies and predecessors,
// dependencies must be propagated to predecessors so the reorder pass cannot
// place a dependency block after a predecessor that would jump over it.
//
// block_8 is the merge of an if/else. It has:
//   dependencies = [block_1, block_4]   (data deps)
//   predecessors = [block_5, block_7]   (control flow edges)
//
// Without propagation, the solver could place block_4 (2nd recv_int) AFTER
// block_5 (the branch), making block_4 unreachable and causing a runtime
// crash when block_8 tries to read %2.
//
// With propagation, block_5 gains block_4 as a dependency, so block_4 must
// appear before block_5 in the output.

// Verify that both recv_int blocks appear before the first cond_br.
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_2:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_1:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_0:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_3:.*]]", deadlines = {}, dependencies = ["[[BLOCK_0]]"], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.recv_int
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_4:.*]]", deadlines = {}, dependencies = ["[[BLOCK_0]]"], predecessors = [], prev_comm = "[[BLOCK_3]]", prev_ent = ""}
// CHECK: qoalahost.recv_int
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_5:.*]]", deadlines = {}, dependencies = ["[[BLOCK_1]]", "[[BLOCK_3]]", "[[BLOCK_4]]"], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: cf.cond_br
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_6:.*]]", deadlines = {}, dependencies = ["[[BLOCK_1]]", "[[BLOCK_2]]", "[[BLOCK_4]]"], predecessors = ["[[BLOCK_5]]"], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_7:.*]]", deadlines = {}, dependencies = ["[[BLOCK_1]]", "[[BLOCK_4]]"], predecessors = ["[[BLOCK_6]]"], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_8:.*]]", deadlines = {}, dependencies = ["[[BLOCK_1]]", "[[BLOCK_4]]"], predecessors = ["[[BLOCK_5]]", "[[BLOCK_7]]"], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_9:.*]]", deadlines = {}, dependencies = ["[[BLOCK_6]]"], predecessors = ["[[BLOCK_8]]"], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_10:.*]]", deadlines = {}, dependencies = ["[[BLOCK_9]]"], predecessors = ["[[BLOCK_9]]"], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_11:.*]]", deadlines = {}, dependencies = ["[[BLOCK_9]]"], predecessors = ["[[BLOCK_10]]", "[[BLOCK_8]]"], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_12:.*]]", deadlines = {}, dependencies = ["[[BLOCK_11]]"], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_13:.*]]", deadlines = {}, dependencies = ["[[BLOCK_0]]", "[[BLOCK_12]]"], predecessors = [], prev_comm = "[[BLOCK_4]]", prev_ent = ""}

module {
  qremote.remote @client
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.request_routine @__qoala_wrapper0() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.eprs %0 {remote = @client}
    netqasm.return %0 : i32
  }
  netqasm.local_routine @__qoala_wrapper1(%arg0: i32) {
    netqasm.z %arg0
    netqasm.return
  }
  netqasm.local_routine @__qoala_wrapper2(%arg0: i32) {
    netqasm.z %arg0
    netqasm.return
  }
  netqasm.local_routine @__qoala_wrapper3(%arg0: i32) {
    netqasm.z %arg0
    netqasm.return
  }
  netqasm.local_routine @__qoala_wrapper4(%arg0: i32) -> i1 {
    %0 = netqasm.measure %arg0 : i1
    netqasm.return %0 : i1
  }
  qoalahost.main_func @server_line_graph() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.remote_id_ref  {classical = true, quantum = true, remote = @client}
    qoalahost.nop_term
  ^bb1:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %c1_i32 = arith.constant 1 : i32
    qoalahost.nop_term
  ^bb2:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.call @__qoala_wrapper0() : () -> i32
  ^bb3:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = ["block_0"], predecessors = [], prev_comm = "", prev_ent = ""}
    %1 = qoalahost.recv_int  {remote = @client} : i32
  ^bb4:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = ["block_0"], predecessors = [], prev_comm = "block_3", prev_ent = ""}
    %2 = qoalahost.recv_int  {remote = @client} : i32
  ^bb5:  // no predecessors
    // With propagation from merge block_8: block_4 added as dependency
    qoalahost.blk_meta  {block_id = "block_5", deadlines = {}, dependencies = ["block_1", "block_3", "block_4"], predecessors = [], prev_comm = "", prev_ent = ""}
    %3 = arith.cmpi eq, %1, %c1_i32 : i32
    cf.cond_br %3, ^bb6, ^bb8
  ^bb6:  // pred: ^bb5
    // With propagation from merge block_8: block_1, block_4 added as dependencies
    qoalahost.blk_meta  {block_id = "block_6", deadlines = {}, dependencies = ["block_1", "block_2", "block_4"], predecessors = ["block_5"], prev_comm = "", prev_ent = ""}
    qoalahost.call @__qoala_wrapper1(%0) : (i32) -> ()
  ^bb7:  // no predecessors
    // With propagation from merge block_8: block_1, block_4 added as dependencies
    qoalahost.blk_meta  {block_id = "block_7", deadlines = {}, dependencies = ["block_1", "block_4"], predecessors = ["block_6"], prev_comm = "", prev_ent = ""}
    cf.br ^bb8
  ^bb8:  // 2 preds: ^bb5, ^bb7
    qoalahost.blk_meta  {block_id = "block_8", deadlines = {}, dependencies = ["block_1", "block_4"], predecessors = ["block_5", "block_7"], prev_comm = "", prev_ent = ""}
    %4 = arith.cmpi eq, %2, %c1_i32 : i32
    cf.cond_br %4, ^bb9, ^bb11
  ^bb9:  // pred: ^bb8
    qoalahost.blk_meta  {block_id = "block_9", deadlines = {}, dependencies = ["block_6"], predecessors = ["block_8"], prev_comm = "", prev_ent = ""}
    qoalahost.call @__qoala_wrapper2(%0) : (i32) -> ()
  ^bb10:  // no predecessors
    // With propagation from merge block_11: block_9 added as dependency
    qoalahost.blk_meta  {block_id = "block_10", deadlines = {}, dependencies = ["block_9"], predecessors = ["block_9"], prev_comm = "", prev_ent = ""}
    cf.br ^bb11
  ^bb11:  // 2 preds: ^bb8, ^bb10
    qoalahost.blk_meta  {block_id = "block_11", deadlines = {}, dependencies = ["block_9"], predecessors = ["block_10", "block_8"], prev_comm = "", prev_ent = ""}
    qoalahost.call @__qoala_wrapper3(%0) : (i32) -> ()
  ^bb12:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_12", deadlines = {}, dependencies = ["block_11"], predecessors = [], prev_comm = "", prev_ent = ""}
    %5 = qoalahost.call @__qoala_wrapper4(%0) : (i32) -> i1
  ^bb13:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_13", deadlines = {}, dependencies = ["block_0", "block_12"], predecessors = [], prev_comm = "block_4", prev_ent = ""}
    %6 = arith.extui %5 : i1 to i32
    qoalahost.send_int %6 {remote = @client} : i32
    qoalahost.return
  }
}
