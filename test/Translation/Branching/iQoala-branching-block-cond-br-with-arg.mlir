// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s

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
// CHECK: ^b0 { type = CL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: %[[HOST_REG1:.*]] = assign_cval() : 0
// CHECK: ^b1 { type = CL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: %[[HOST_REG2:.*]] = assign_cval() : 0
// CHECK-NEXT: %[[HOST_REG3:.*]] = assign_cval() : 1
// CHECK-NEXT: %[[HOST_REG4:.*]] = assign_cval() : 3
// CHECK-NEXT: %[[HOST_REG5:.*]] = assign_cval() : 2
// Trick: we eagerly copy the "true" value in the register reserved for the block argument
// CHECK-NEXT: %[[HOST_REG0:.*]] = copy_cval(%[[HOST_REG2]])
// If the next conditional branch is true, then HOST_REG0 contains the value for the true branch
// CHECK-NEXT: beq(%[[HOST_REG4]], %[[HOST_REG5]]) : b2
// If the next conditional branch is false, then we *overwrite* HOST_REG0 with the false value for the block argument.
// CHECK-NEXT: %[[HOST_REG0]] = copy_cval(%[[HOST_REG3]])
// So, at this point (branch false) HOST_REG0 contains the value for the false branch
// CHECK-NEXT: jump() : b2
// CHECK: ^b2 { type = CL; predecessors = [b1]; dependencies = [b1]; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: jump() : b3
// CHECK: ^b3 { type = CL; predecessors = [b2]; dependencies = [b2]; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: send_cmsg(%[[HOST_REG1]], %[[HOST_REG0]])

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
    cf.cond_br %jump, ^bb2(%zero : i32), ^bb2(%one : i32)
  ^bb2(%7: i32):
    qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = ["block_1"], predecessors = ["block_1"], prev_comm = "", prev_ent = ""}
    cf.br ^bb4
  ^bb4:
    qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = ["block_2"], predecessors = ["block_2"], prev_comm = "", prev_ent = ""}
    qoalahost.send_int %7 {remote = @Charlie} : i32
    qoalahost.nop_term
  ^bb5:
    qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = [], predecessors = ["block_3"], prev_comm = "", prev_ent = ""}
    qoalahost.return
  }
}
