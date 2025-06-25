// RUN: qoala-opt %s --qoalahost-show-analysis-qmem-eff | FileCheck %s
// CHECK: Efficiency = 0.000000e+00

module {
    qremote.remote @Bob

    netqasm.request_routine @req1() -> i1 {
      %vqubit = netqasm.qalloc : i32
      %m = netqasm.eprs_measure %vqubit {remote = @Bob} : i1
      netqasm.return %m : i1
    }

    qoalahost.main_func @main() {
      qoalahost.blk_meta  {block_id = "block_0", predecessors = []}
      %zero = arith.constant 0 : index
      qoalahost.nop_term
    
    ^bb1:
      qoalahost.blk_meta  {block_id = "block_1", predecessors = []}
      %bit = qoalahost.call @req1() : () -> i1
    
    ^bb2:
      qoalahost.blk_meta  {block_id = "block_2", predecessors = []}
      qoalahost.return
    
    }

}