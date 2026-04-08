// RUN: qoala-opt %s --qoalahost-show-analysis-esp | FileCheck %s
// CHECK:  [ESP]: {{8\.6.*e-01}}

module {
  qremote.remote @Alice
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.request_routine @__qoala_wrapper0() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.eprs %0 {remote = @Alice}
    netqasm.return %0 : i32
  }
  netqasm.local_routine @__qoala_wrapper1(%arg0: i32) {
    netqasm.z %arg0
    netqasm.return
  }
  netqasm.local_routine @__qoala_wrapper2(%arg0: i32) {
    netqasm.x %arg0
    netqasm.return
  }
  netqasm.local_routine @__qoala_wrapper3(%arg0: i32) {
    netqasm.x %arg0
    netqasm.return
  }
  netqasm.local_routine @__qoala_wrapper4(%arg0: i32) {
    netqasm.z %arg0
    netqasm.return
  }
  netqasm.local_routine @__qoala_wrapper5(%arg0: i32) {
    netqasm.y %arg0
    netqasm.return
  }
  netqasm.local_routine @__qoala_wrapper6(%arg0: i32) {
    netqasm.x %arg0
    netqasm.return
  }
  netqasm.local_routine @__qoala_wrapper7(%arg0: i32) {
    netqasm.x %arg0
    netqasm.return
  }
  netqasm.local_routine @__qoala_wrapper8(%arg0: i32) {
    netqasm.z %arg0
    netqasm.return
  }
  netqasm.local_routine @__qoala_wrapper9(%arg0: i32) {
    netqasm.x %arg0
    netqasm.return
  }
  netqasm.local_routine @__qoala_wrapper10(%arg0: i32) -> i1 {
    %0 = netqasm.measure %arg0 : i1
    netqasm.return %0 : i1
  }
  qoalahost.main_func @teleport_bob() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.remote_id_ref  {classical = true, quantum = true, remote = @Alice}
    qoalahost.nop_term
  ^bb1:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %c1_i32 = arith.constant 1 : i32
    qoalahost.nop_term
  ^bb2:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.call @__qoala_wrapper0() : () -> i32
  ^bb3:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.nop_term
  ^bb4:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = ["block_0"], predecessors = [], prev_comm = "", prev_ent = ""}
    %1 = qoalahost.recv_int  {remote = @Alice} : i32
  ^bb5:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_5", deadlines = {}, dependencies = ["block_0"], predecessors = [], prev_comm = "block_4", prev_ent = ""}
    %2 = qoalahost.recv_int  {remote = @Alice} : i32
  ^bb6:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_6", deadlines = {}, dependencies = ["block_1", "block_2", "block_4", "block_5"], predecessors = [], prev_comm = "", prev_ent = ""}
    %3 = arith.cmpi eq, %1, %c1_i32 : i32
    cf.cond_br %3, ^bb7, ^bb10
  ^bb7:  // pred: ^bb6
    qoalahost.blk_meta  {block_id = "block_7", deadlines = {}, dependencies = ["block_2"], predecessors = ["block_6"], prev_comm = "", prev_ent = ""}
    qoalahost.call @__qoala_wrapper1(%0) : (i32) -> ()
  ^bb8:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_8", deadlines = {}, dependencies = ["block_7"], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.call @__qoala_wrapper2(%0) : (i32) -> ()
  ^bb9:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_9", deadlines = {}, dependencies = ["block_1", "block_5"], predecessors = ["block_8"], prev_comm = "", prev_ent = ""}
    cf.br ^bb15
  ^bb10:  // pred: ^bb6
    qoalahost.blk_meta  {block_id = "block_10", deadlines = {}, dependencies = ["block_8"], predecessors = ["block_6"], prev_comm = "", prev_ent = ""}
    qoalahost.call @__qoala_wrapper3(%0) : (i32) -> ()
  ^bb11:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_11", deadlines = {}, dependencies = ["block_10"], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.call @__qoala_wrapper4(%0) : (i32) -> ()
  ^bb12:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_12", deadlines = {}, dependencies = ["block_11"], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.call @__qoala_wrapper5(%0) : (i32) -> ()
  ^bb13:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_13", deadlines = {}, dependencies = ["block_12"], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.call @__qoala_wrapper6(%0) : (i32) -> ()
  ^bb14:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_14", deadlines = {}, dependencies = ["block_1", "block_5"], predecessors = ["block_13"], prev_comm = "", prev_ent = ""}
    cf.br ^bb15
  ^bb15:  // 2 preds: ^bb9, ^bb14
    qoalahost.blk_meta  {block_id = "block_15", deadlines = {}, dependencies = ["block_1", "block_13", "block_5"], predecessors = ["block_14", "block_9"], prev_comm = "", prev_ent = ""}
    %4 = arith.cmpi eq, %2, %c1_i32 : i32
    cf.cond_br %4, ^bb16, ^bb19
  ^bb16:  // pred: ^bb15
    qoalahost.blk_meta  {block_id = "block_16", deadlines = {}, dependencies = ["block_13"], predecessors = ["block_15"], prev_comm = "", prev_ent = ""}
    qoalahost.call @__qoala_wrapper7(%0) : (i32) -> ()
  ^bb17:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_17", deadlines = {}, dependencies = ["block_16"], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.call @__qoala_wrapper8(%0) : (i32) -> ()
  ^bb18:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_18", deadlines = {}, dependencies = ["block_17"], predecessors = ["block_17"], prev_comm = "", prev_ent = ""}
    cf.br ^bb21
  ^bb19:  // pred: ^bb15
    qoalahost.blk_meta  {block_id = "block_19", deadlines = {}, dependencies = ["block_17"], predecessors = ["block_15"], prev_comm = "", prev_ent = ""}
    qoalahost.call @__qoala_wrapper9(%0) : (i32) -> ()
  ^bb20:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_20", deadlines = {}, dependencies = ["block_19"], predecessors = ["block_19"], prev_comm = "", prev_ent = ""}
    cf.br ^bb21
  ^bb21:  // 2 preds: ^bb18, ^bb20
    qoalahost.blk_meta  {block_id = "block_21", deadlines = {}, dependencies = ["block_19"], predecessors = [], prev_comm = "", prev_ent = ""}
    %5 = qoalahost.call @__qoala_wrapper10(%0) : (i32) -> i1
  ^bb22:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_22", deadlines = {}, dependencies = ["block_21"], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.return %5 : i1
  }
}
