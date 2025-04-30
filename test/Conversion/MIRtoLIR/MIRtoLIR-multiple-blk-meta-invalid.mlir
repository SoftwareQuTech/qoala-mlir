// RUN: qoala-opt %s --lower-qoala-mir-to-lir --verify-diagnostics
// This test check that the conversion fails, due to a block having multiple qoalahost.blk_meta oeprations
// We need to add the "--verify-diagnostics" option to check for the error
module {
  // expected-error@+1 {{'qoalahost.main_func' op each block must contain exactly one 'qoalahost.blk_meta' operation, but found multiple.}}
  qoalahost.main_func @test_block_without_blk_meta_op() {
    qoalahost.blk_meta {block_id = "block_0", predecessors = []}
    qoalahost.blk_meta {block_id = "block_0", predecessors = []}
    %cst = arith.constant 0 : i32
    qoalahost.nop_term
    ^bb1:
        qoalahost.blk_meta {block_id = "block_1", predecessors = []}
        qoalahost.return
  }
}
