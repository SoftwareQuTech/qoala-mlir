// RUN: qoala-translate %s --mlir-to-iqoala --verify-diagnostics

module {
  // expected-warning@+1 {{Remote with name 'Bob' is not used. The remote reference was not added as an iQoala parameter.}}
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @__qoala_wrapper0() -> i32 {
    %cst = arith.constant 1 : i32
    netqasm.return %cst : i32
  }
  qoalahost.main_func @test_host_ret_one_val() -> i32 {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %cst = arith.constant 3 : i32
    qoalahost.nop_term
  ^bb1:
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.call @__qoala_wrapper0() : () -> i32
  ^bb2:
    qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = ["block_0", "block_1"], predecessors = [], prev_comm = "", prev_ent = ""}
    %1 = arith.addi %0, %cst : i32
    qoalahost.return %1 : i32
  }
}
