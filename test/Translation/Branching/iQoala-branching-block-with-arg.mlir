// XXXXX qoala-translate %s --mlir-to-iqoala | FileCheck %s

// This test is a bit fragile. In the QoalaHost section it is not possible
// to capture the name of the block to branch to, since it is usually defined
// AFTER the place where it is used.
// This is why all the branching destinations are "hard-coded"

// CHECK: META_START
// CHECK-NEXT: name: test_block_with_args
// CHECK-NEXT: parameters:
// CHECK-NEXT: csockets:
// CHECK-NEXT: epr_sockets:
// CHECK-NEXT: META_END
// CHECK-NEXT: ^b0 { type = CL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: %[[HOST_REG0:.*]] = assign_cval() : 3
// CHECK-NEXT: %[[HOST_REG1:.*]] = assign_cval() : 2
// CHECK-NEXT: beq(%[[HOST_REG0]], %[[HOST_REG1]]) : b1
// CHECK-NEXT: jump() : b2
// CHECK: ^b1 { type = CL; predecessors = [b0]; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: jump() : b3
// CHECK: ^b2 { type = CL; predecessors = [b0]; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: jump() : b3
// CHECK: ^b3 { type = QL; predecessors = [b1, b2]; dependencies = [b0]; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: tuple<%[[HOST_REG2:.*]]> = run_subroutine(tuple<%[[HOST_REG0]]>) : __qoala_wrapper0
// CHECK: ^b4 { type = CL; predecessors = []; dependencies = [b0, b3]; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: %[[HOST_REG3:.*]] = add_cval_c(%[[HOST_REG2]], %[[HOST_REG1]])

module {
  qremote.remote @Charlie
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> i1
  qoalahost.main_func @test_block_with_args() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.remote_id_ref  {classical = true, quantum = true, remote = @Charlie}
    qoalahost.nop_term
  ^bb1:
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %zero = arith.constant 0 : i32
    %one = arith.constant 1 : i32
    %cstA = arith.constant 3 : i32
    %cstB = arith.constant 2 : i32
    %jump = arith.cmpi eq, %cstA, %cstB : i32
    cf.cond_br %jump, ^bb1, ^bb2
  ^bb2:
    qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = [], predecessors = ["block_1"], prev_comm = "", prev_ent = ""}
    cf.br ^bb4(%zero : i32)
  ^bb3:
    qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = [], predecessors = ["block_1"], prev_comm = "", prev_ent = ""}
    cf.br ^bb4(%one : i32)
  ^bb4(%7: i32):
    qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = ["block_1"], predecessors = ["block_2", "block_3"], prev_comm = "", prev_ent = ""}
    cf.br ^bb5
  ^bb5:
    qoalahost.blk_meta  {block_id = "block_5", deadlines = {}, dependencies = ["block_4"], predecessors = ["block_4"], prev_comm = "", prev_ent = ""}
    qoalahost.send_int %7 {remote = @Charlie} : i32
    qoalahost.nop_term
  ^bb6:
    qoalahost.blk_meta  {block_id = "block_6", deadlines = {}, dependencies = [], predecessors = ["block_5"], prev_comm = "", prev_ent = ""}
    qoalahost.return
  }
}
