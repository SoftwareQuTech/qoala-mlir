// RUN: qoala-opt %s --verify-diagnostics
// This test check that the conversion fails, due to a block not being in a sane order
// We need to add the "--verify-diagnostics" option to check for the error

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.request_routine @__qoala_wrapper0() {
    %0 = netqasm.qalloc  : i32
    netqasm.eprs %0 {remote = @Bob}
    netqasm.return
  }
  netqasm.request_routine @__qoala_wrapper1() {
    %0 = netqasm.qalloc  : i32
    netqasm.eprs %0 {remote = @Bob}
    netqasm.return
  }
  qoalahost.main_func @test_prev_ent_order_invalid() {
    // expected-error@+1 {{'qoalahost.blk_meta' op contains a previous ent precedence before its decalration.}}
    qoalahost.blk_meta  {block_id = "block_0", dependencies = [], predecessors = [], prev_comm = "", prev_ent = "block_1"}
    qoalahost.call @__qoala_wrapper0() : () -> ()
  ^bb1: 
    qoalahost.blk_meta  {block_id = "block_1", dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.call @__qoala_wrapper1() : () -> ()
  ^bb2:
    qoalahost.blk_meta  {block_id = "block_3", dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.return
  }
}
