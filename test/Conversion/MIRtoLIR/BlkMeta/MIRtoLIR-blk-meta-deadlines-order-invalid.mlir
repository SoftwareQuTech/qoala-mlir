// RUN: qoala-opt %s --verify-diagnostics
// This test check that the conversion fails, due to a block not being in a sane order
// We need to add the "--verify-diagnostics" option to check for the error

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.request_routine @__qoala_wrapper0() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.eprs %0 {remote = @Bob}
    netqasm.return %0 : i32
  }
  netqasm.local_routine @__qoala_wrapper1(%arg0: i32, %arg1: i32) -> i1 {
    netqasm.cnot %arg0, %arg1
    %0 = netqasm.measure %arg0 : i1
    netqasm.return %0 : i1
  }
  netqasm.local_routine @__qoala_wrapper2() -> (i32, i1) {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    %1 = netqasm.measure %0 : i1
    netqasm.return %0, %1 : i32, i1
  }
  qoalahost.main_func @test_add_block_deps() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.remote_id_ref  {classical = false, quantum = true, remote = @Bob}
    qoalahost.nop_term
  ^bb1:
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.call @__qoala_wrapper0() : () -> i32
  ^bb2:
    // expected-error@+1 {{'qoalahost.blk_meta' op contains a block identifier in deadlines before its declaration.}}
    qoalahost.blk_meta  {block_id = "block_2", deadlines = {"block_1" = 10, "block_3" = 20}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %1 = qoalahost.call @__qoala_wrapper1(%0, %2#0) : (i32, i32) -> i1
  ^bb3:
    qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %2:2 = qoalahost.call @__qoala_wrapper2() : () -> (i32, i1)
  ^bb4:
    qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.return
  }
}
