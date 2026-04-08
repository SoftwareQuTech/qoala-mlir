// RUN: qoala-opt %s --qoalahost-show-analysis-qmem-eff | FileCheck %s
// CHECK: [QMem Efficiency]:0.000000e+00

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @__qoala_wrapper0() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.return %0 : i32
  }
  netqasm.local_routine @__qoala_wrapper1() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.return %0 : i32
  }
  netqasm.local_routine @__qoala_wrapper2(%arg0: i32) -> i1 {
    %0 = netqasm.measure %arg0 : i1
    netqasm.return %0 : i1
  }
  netqasm.local_routine @__qoala_wrapper3() {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.return
  }
  netqasm.local_routine @__qoala_wrapper4(%arg0: i32) -> i1 {
    %0 = netqasm.measure %arg0 : i1
    netqasm.return %0 : i1
  }
  netqasm.request_routine @__qoala_wrapper5() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.eprs %0 {remote = @Bob}
    netqasm.return %0 : i32
  }
  netqasm.local_routine @__qoala_wrapper6(%arg0: i32) {
    netqasm.x %arg0
    netqasm.return
  }
  netqasm.local_routine @__qoala_wrapper7(%arg0: i32) {
    netqasm.z %arg0
    netqasm.return
  }
  netqasm.local_routine @__qoala_wrapper8(%arg0: i32) {
    netqasm.y %arg0
    netqasm.return
  }
  netqasm.local_routine @__qoala_wrapper9(%arg0: i32) -> i1 {
    %0 = netqasm.measure %arg0 : i1
    netqasm.return %0 : i1
  }
  qoalahost.main_func @qmem_eff() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.remote_id_ref  {classical = true, quantum = true, remote = @Bob}
    qoalahost.nop_term
  ^bb1:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %c1_i32 = arith.constant 1 : i32
    %true = arith.constant true
    qoalahost.nop_term
  ^bb2:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.call @__qoala_wrapper0() : () -> i32
  ^bb3:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %1 = qoalahost.call @__qoala_wrapper1() : () -> i32
  ^bb4:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.nop_term
  ^bb5:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_5", deadlines = {}, dependencies = ["block_0"], predecessors = [], prev_comm = "", prev_ent = ""}
    %2 = qoalahost.recv_int  {remote = @Bob} : i32
  ^bb6:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_6", deadlines = {}, dependencies = ["block_1", "block_2", "block_3", "block_5"], predecessors = [], prev_comm = "", prev_ent = ""}
    %4 = arith.cmpi eq, %2, %c1_i32 : i32
    cf.cond_br %4, ^bb7, ^bb9
  ^bb7:  // pred: ^bb7
    qoalahost.blk_meta  {block_id = "block_7", deadlines = {}, dependencies = ["block_3"], predecessors = ["block_6"], prev_comm = "", prev_ent = ""}
    %5 = qoalahost.call @__qoala_wrapper2(%1) : (i32) -> i1
  ^bb8:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_8", deadlines = {}, dependencies = ["block_7"], predecessors = ["block_7"], prev_comm = "", prev_ent = ""}
    cf.br ^bb10(%5 : i1)
  ^bb9:  // pred: ^bb7
    qoalahost.blk_meta  {block_id = "block_9", deadlines = {}, dependencies = ["block_1"], predecessors = ["block_6"], prev_comm = "", prev_ent = ""}
    cf.br ^bb10(%true : i1)
  ^bb10(%6: i1):  // 2 preds: ^bb9, ^bb10
    qoalahost.blk_meta  {block_id = "block_10", deadlines = {}, dependencies = [], predecessors = ["block_8", "block_9"], prev_comm = "", prev_ent = ""}
    cf.br ^bb11
  ^bb11:  // pred: ^bb11
    qoalahost.blk_meta  {block_id = "block_11", deadlines = {}, dependencies = [], predecessors = ["block_10"], prev_comm = "", prev_ent = ""}
    qoalahost.call @__qoala_wrapper3() : () -> ()
  ^bb12:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_12", deadlines = {}, dependencies = ["block_2"], predecessors = [], prev_comm = "", prev_ent = ""}
    %7 = qoalahost.call @__qoala_wrapper4(%0) : (i32) -> i1
  ^bb13:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_13", deadlines = {}, dependencies = ["block_12"], predecessors = [], prev_comm = "", prev_ent = ""}
    %3 = qoalahost.call @__qoala_wrapper5() : () -> i32
  ^bb14:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_14", deadlines = {}, dependencies = ["block_1", "block_13"], predecessors = [], prev_comm = "", prev_ent = ""}
    %8 = arith.extui %6 : i1 to i32
    %9 = arith.cmpi eq, %8, %c1_i32 : i32
    cf.cond_br %9, ^bb15, ^bb18
  ^bb15:  // pred: ^bb14
    qoalahost.blk_meta  {block_id = "block_15", deadlines = {}, dependencies = ["block_13"], predecessors = ["block_14"], prev_comm = "", prev_ent = ""}
    qoalahost.call @__qoala_wrapper6(%3) : (i32) -> ()
  ^bb16:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_16", deadlines = {}, dependencies = ["block_15"], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.call @__qoala_wrapper7(%3) : (i32) -> ()
  ^bb17:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_17", deadlines = {}, dependencies = ["block_16"], predecessors = ["block_16"], prev_comm = "", prev_ent = ""}
    cf.br ^bb20
  ^bb18:  // pred: ^bb14
    qoalahost.blk_meta  {block_id = "block_18", deadlines = {}, dependencies = ["block_16"], predecessors = ["block_14"], prev_comm = "", prev_ent = ""}
    qoalahost.call @__qoala_wrapper8(%3) : (i32) -> ()
  ^bb19:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_19", deadlines = {}, dependencies = ["block_18"], predecessors = ["block_18"], prev_comm = "", prev_ent = ""}
    cf.br ^bb20
  ^bb20:  // 2 preds: ^bb17, ^bb19
    qoalahost.blk_meta  {block_id = "block_20", deadlines = {}, dependencies = ["block_18"], predecessors = ["block_17", "block_19"], prev_comm = "", prev_ent = ""}
    %10 = qoalahost.call @__qoala_wrapper9(%3) : (i32) -> i1
  ^bb21:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_21", deadlines = {}, dependencies = ["block_20"], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.return %10 : i1
  }
}