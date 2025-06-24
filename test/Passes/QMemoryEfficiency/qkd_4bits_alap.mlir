// RUN: qoala-opt %s --qoalahost-show-analysis | FileCheck %s
// CHECK: Efficiency = 7.500000e-01

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
      qoalahost.blk_meta  {block_id = "block_0", predecessors = []}
      %zero = arith.constant 0 : index
      qoalahost.nop_term
    
    ^bb1:
      qoalahost.blk_meta  {block_id = "block_1", predecessors = []}
      %vqubit1 = qoalahost.call @req1() : () -> i32
    
    ^bb2:
      qoalahost.blk_meta  {block_id = "block_2", predecessors = ["block_1"]}
      %m1 = qoalahost.call @subrt1(%vqubit1) : (i32) -> i1
    
    ^bb3:
      qoalahost.blk_meta  {block_id = "block_3", predecessors = []}
      %vqubit2 = qoalahost.call @req2() : () -> i32
    
    ^bb4:
      qoalahost.blk_meta  {block_id = "block_4", predecessors = ["block_3"]}
      %m2 = qoalahost.call @subrt2(%vqubit2) : (i32) -> i1
    
    ^bb5:
      qoalahost.blk_meta  {block_id = "block_5", predecessors = []}
      %vqubit3 = qoalahost.call @req3() : () -> i32
    
    ^bb6:
      qoalahost.blk_meta  {block_id = "block_6", predecessors = ["block_5"]}
      %m3 = qoalahost.call @subrt3(%vqubit3) : (i32) -> i1
    
    ^bb7:
      qoalahost.blk_meta  {block_id = "block_7", predecessors = []}
      %vqubit4 = qoalahost.call @req4() : () -> i32
    
    ^bb8:
      qoalahost.blk_meta  {block_id = "block_8", predecessors = ["block_7"]}
      %m4 = qoalahost.call @subrt4(%vqubit4) : (i32) -> i1
    
    ^bb9:
      qoalahost.blk_meta  {block_id = "block_9", predecessors = []}
      qoalahost.return
    
    }

}