// RUN: qoala-opt %s --lower-qoala-mir-to-lir --verify-diagnostics
// This test check that the conversion fails, due to a block having multiple qoalahost.blk_meta oeprations
// We need to add the "--verify-diagnostics" option to check for the error
module {
  qoalahost.main_func @test_block_without_blk_meta_op() {
    %cst = arith.constant 0 : i32
    // expected-error@+1 {{'qoalahost.blk_meta' op must be the first operation in each block.}}
    qoalahost.blk_meta {block_id = "block_0", dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.nop_term
    ^bb1:
        qoalahost.blk_meta {block_id = "block_1", dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
        qoalahost.return
  }
}
