// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s

// This test is a bit fragile. In the QoalaHost section it is not possible
// to capture the name of the block to branch to, since it is usually defined
// AFTER the place where it is used.
// This is why all the branching destinations are "hard-coded"

// CHECK: META START
// CHECK-NEXT: name: test_branching_slt
// CHECK-NEXT: parameters: Bob_id
// CHECK-NEXT: csockets: 0 -> Bob
// CHECK-NEXT: epr_sockets: 0 -> Bob
// CHECK-NEXT: META END
// CHECK-NEXT: ^b0 { type = CL, predecessors = [] }
// CHECK-NEXT: %[[HOST_REG0:.*]] = assign_cval() : 3
// CHECK-NEXT: %[[HOST_REG1:.*]] = assign_cval() : 2
// CHECK-NEXT: blt(%[[HOST_REG0]], %[[HOST_REG1]]) : b1
// CHECK-NEXT: jump() : b2
// CHECK: ^b1 { type = CL, predecessors = [] }
// CHECK-NEXT: jump() : b3
// CHECK: ^b2 { type = CL, predecessors = [] }
// CHECK-NEXT: jump() : b3
// CHECK: ^b3 { type = QL, predecessors = [b0] }
// CHECK-NEXT: %[[HOST_REG2:.*]] = run_subroutine(tuple<%[[HOST_REG0]]>) : __qoala_wrapper0
// CHECK: ^b4 { type = CL, predecessors = [b0, b3] }
// CHECK-NEXT: %[[HOST_REG3:.*]] = add_cval_c(%[[HOST_REG2]], %[[HOST_REG1]])

// CHECK: SUBROUTINE __qoala_wrapper0
// CHECK-NEXT: params: p0
// CHECK-NEXT: returns: m0
// CHECK-NEXT: uses:
// CHECK-NEXT: keeps:
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: load R[[ARG0_REG:.*]] @input[0]
// CHECK-NEXT: set C[[C_REG1:.*]] 10
// CHECK-NEXT: blt C[[C_REG1]] R[[ARG0_REG]] 2
// CHECK-NEXT: jmp 3
// CHECK-NEXT: add C[[C_REG2:.*]] R[[ARG0_REG]] C[[C_REG1]]
// CHECK-NEXT: jmp 3
// CHECK-NEXT: sub C[[C_REG3:.*]] R[[ARG0_REG]] C[[C_REG1]]
// CHECK-NEXT: jmp 1
// CHECK-NEXT: mul C[[C_REG4:.*]] R[[ARG0_REG]] C[[C_REG1]]
// CHECK-NEXT: store C[[C_REG4]] @output[0]
// CHECK-NEXT: NETQASM_END

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> i1
  netqasm.local_routine @__qoala_wrapper0(%arg0: i32) -> i32 {
    %cstA = arith.constant 10 : i32
    // In NetQASM, there is no "branch on less than or equal" (blt) instruction,
    // so using "arith.cmpi sle" would yield and error
    %jump_loc = arith.cmpi slt, %cstA, %arg0 : i32
    cf.cond_br %jump_loc, ^bb1, ^bb2
  ^bb1:
    %1 = arith.addi %arg0, %cstA : i32
    cf.br ^bb3
  ^bb2:
    %2 = arith.subi %arg0, %cstA : i32
    cf.br ^bb3
  ^bb3:
    %3 = arith.muli %arg0, %cstA : i32
    netqasm.return %3 : i32
  }
  qoalahost.main_func @test_branching_slt() {
    qoalahost.blk_meta  {block_id = "block_0", predecessors = []}
    %cstA = arith.constant 3 : i32
    %cstB = arith.constant 2 : i32
    %jump = arith.cmpi slt, %cstA, %cstB : i32
    cf.cond_br %jump, ^bb1, ^bb2
  ^bb1:
    qoalahost.blk_meta  {block_id = "block_1", predecessors = []}
    cf.br ^bb3
  ^bb2:
    qoalahost.blk_meta  {block_id = "block_2", predecessors = []}
    cf.br ^bb3
  ^bb3:
    qoalahost.blk_meta  {block_id = "block_3", predecessors = ["block_0"]}
    %0 = qoalahost.call @__qoala_wrapper0(%cstA) : (i32) -> i32
  ^bb4:
    qoalahost.blk_meta  {block_id = "block_4", predecessors = ["block_0", "block_3"]}
    %1 = arith.addi %0, %cstB : i32
    qoalahost.return
  }
}
