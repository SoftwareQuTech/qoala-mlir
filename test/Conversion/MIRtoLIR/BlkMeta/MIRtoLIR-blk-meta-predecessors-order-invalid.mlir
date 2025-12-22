// RUN: qoala-opt %s --verify-diagnostics
// This test check that the conversion fails, due to a block not being in a sane order
// We need to add the "--verify-diagnostics" option to check for the error

module {
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> i1
  netqasm.local_routine @__qoala_wrapper0(%arg0: i32) -> i32 {
    %cstA = arith.constant 10 : i32
    cf.br ^bb2
  ^bb1:
    %1 = arith.addi %arg0, %cstA : i32
    cf.br ^bb3
  ^bb2:
    %2 = arith.subi %arg0, %cstA : i32
    cf.br ^bb1
  ^bb3:
    %3 = arith.muli %arg0, %cstA : i32
    netqasm.return %3 : i32
  }
  qoalahost.main_func @test_branching_unconditional() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %cstA = arith.constant 3 : i32
    %cstB = arith.constant 5 : i32
    cf.br ^bb2
  ^bb1:
    // expected-error@+1 {{'qoalahost.blk_meta' op contains a predecessor before its declaration: 'block_2'}}
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = ["block_2"], prev_comm = "", prev_ent = ""}
    cf.br ^bb3
  ^bb2:
    qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = [], predecessors = ["block_0"], prev_comm = "", prev_ent = ""}
    cf.br ^bb1
  ^bb3:
    qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = ["block_0"], predecessors = ["block_1"], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.call @__qoala_wrapper0(%cstA) : (i32) -> i32
  ^bb4:
    qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = ["block_0", "block_3"], predecessors = [], prev_comm = "", prev_ent = ""}
    %1 = arith.addi %0, %cstB : i32
    qoalahost.return
  }
}
