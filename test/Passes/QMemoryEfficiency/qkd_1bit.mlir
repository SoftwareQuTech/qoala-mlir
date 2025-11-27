// RUN: qoala-opt %s --qoalahost-show-analysis-qmem-eff | FileCheck %s
// CHECK: [QMem Efficiency]:0.000000e+00

module {
    qremote.remote @Bob

    netqasm.request_routine @req1() -> i1 {
      %vqubit = netqasm.qalloc : i32
      netqasm.eprs %vqubit {remote = @Bob}
      %m = netqasm.measure %vqubit : i1
      netqasm.return %m : i1
    }

    qoalahost.main_func @test_qkd_1bit_qmem() {
      qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
      %bit = qoalahost.call @req1() : () -> i1
    
    ^bb1:
      qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
      qoalahost.return
    }

}