// RUN: qoala-opt %s --verify-diagnostics
// This test check that the conversion fails, due to a block not being in a sane order
// We need to add the "--verify-diagnostics" option to check for the error

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  qoalahost.main_func @test_add_cc_block_deps() {
    // expected-error@+1 {{'qoalahost.blk_meta' op contains a previous comm precedence before its decalration.}}
    qoalahost.blk_meta  {block_id = "block_0", dependencies = [], predecessors = [], prev_comm = "block_1", prev_ent = ""}
    %cst = arith.constant dense<2> : tensor<1xi32>
    %cst_0 = arith.constant dense<2.120000e+01> : tensor<1xf32>
    qoalahost.send_ints %cst {remote = @Bob} : tensor<1xi32>
    qoalahost.nop_term 
  ^bb1:
    qoalahost.blk_meta  {block_id = "block_1", dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.recv_floats  {length = 2 : i32, remote = @Bob} : tensor<2xf32>
  ^bb2:
    qoalahost.return
  }
}
