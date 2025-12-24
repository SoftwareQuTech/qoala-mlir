// RUN: qoala-opt %s --qoalahost-show-analysis-qmem-eff | FileCheck %s
// CHECK: [QMem Efficiency]:7.500000e-01

module {
    qremote.remote @Bob

    netqasm.request_routine @req1() -> i32 {
      %vqubit = netqasm.qalloc : i32
      netqasm.eprs %vqubit {remote = @Bob}
      netqasm.return %vqubit : i32
    }

    netqasm.request_routine @req2() -> i32 {
      %vqubit = netqasm.qalloc : i32
      netqasm.eprs %vqubit {remote = @Bob}
      netqasm.return %vqubit : i32
    }

    netqasm.request_routine @req3() -> i32 {
      %vqubit = netqasm.qalloc : i32
      netqasm.eprs %vqubit {remote = @Bob}
      netqasm.return %vqubit : i32
    }

    netqasm.request_routine @req4() -> i32 {
      %vqubit = netqasm.qalloc : i32
      netqasm.eprs %vqubit {remote = @Bob}
      netqasm.return %vqubit : i32
    }

    netqasm.local_routine @subrt1(%vqubit: i32) -> i1 {
      %m = netqasm.measure %vqubit : i1
      netqasm.return %m : i1
    }

    netqasm.local_routine @subrt2(%vqubit: i32) -> i1 {
      %m = netqasm.measure %vqubit : i1
      netqasm.return %m : i1
    }

    netqasm.local_routine @subrt3(%vqubit: i32) -> i1 {
      %m = netqasm.measure %vqubit : i1
      netqasm.return %m : i1
    }

    netqasm.local_routine @subrt4(%vqubit: i32) -> i1 {
      %m = netqasm.measure %vqubit : i1
      netqasm.return %m : i1
    }

    qoalahost.main_func @main() {
      qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
      qoalahost.remote_id_ref  {classical = false, quantum = true, remote = @Bob}
      qoalahost.nop_term

    ^bb1:
      qoalahost.blk_meta  {block_id = "block_1", predecessors = [], deadlines = {}, dependencies = [], prev_comm = "", prev_ent = ""}
      %zero = arith.constant 0 : index
      qoalahost.nop_term
    
    ^bb2:
      qoalahost.blk_meta  {block_id = "block_2", predecessors = [], deadlines = {}, dependencies = [], prev_comm = "", prev_ent = ""}
      %vqubit1 = qoalahost.call @req1() : () -> i32
    
    ^bb3:
      qoalahost.blk_meta  {block_id = "block_3", predecessors = ["block_2"], deadlines = {}, dependencies = ["block_2"], prev_comm = "", prev_ent = ""}
      %m1 = qoalahost.call @subrt1(%vqubit1) : (i32) -> i1
    
    ^bb4:
      qoalahost.blk_meta  {block_id = "block_4", predecessors = [], deadlines = {}, dependencies = [], prev_comm = "", prev_ent = "block_2"}
      %vqubit2 = qoalahost.call @req2() : () -> i32
    
    ^bb5:
      qoalahost.blk_meta  {block_id = "block_5", predecessors = ["block_4"], deadlines = {}, dependencies = ["block_4"], prev_comm = "", prev_ent = ""}
      %m2 = qoalahost.call @subrt2(%vqubit2) : (i32) -> i1
    
    ^bb6:
      qoalahost.blk_meta  {block_id = "block_6", predecessors = [], deadlines = {}, dependencies = [], prev_comm = "", prev_ent = "block_4"}
      %vqubit3 = qoalahost.call @req3() : () -> i32
    
    ^bb7:
      qoalahost.blk_meta  {block_id = "block_7", predecessors = ["block_6"], deadlines = {}, dependencies = ["block_6"], prev_comm = "", prev_ent = ""}
      %m3 = qoalahost.call @subrt3(%vqubit3) : (i32) -> i1
    
    ^bb8:
      qoalahost.blk_meta  {block_id = "block_8", predecessors = [], deadlines = {}, dependencies = [], prev_comm = "", prev_ent = "block_6"}
      %vqubit4 = qoalahost.call @req4() : () -> i32
    
    ^bb9:
      qoalahost.blk_meta  {block_id = "block_9", predecessors = ["block_8"], deadlines = {}, dependencies = ["block_8"], prev_comm = "", prev_ent = ""}
      %m4 = qoalahost.call @subrt4(%vqubit4) : (i32) -> i1
    
    ^bb10:
      qoalahost.blk_meta  {block_id = "block_10", predecessors = [], deadlines = {}, dependencies = [], prev_comm = "", prev_ent = ""}
      qoalahost.return
    }
}