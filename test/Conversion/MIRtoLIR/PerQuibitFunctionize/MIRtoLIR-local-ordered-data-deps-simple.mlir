// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

// CHECK: module
module {
  // CHECK-LABEL: netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)

  // CHECK: netqasm.local_routine @[[WRAPPER0:.*]](%[[C3_ARG:.+]]: i32, %[[C2_ARG:.+]]: i32, %[[C1_ARG:.+]]: i32) -> i1
  // CHECK-NEXT: %[[REG0_0:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.init %[[REG0_0]]
  // CHECK-NEXT: netqasm.rot_x %[[REG0_0]], %[[C3_ARG]], %[[C2_ARG]]
  // CHECK-NEXT: netqasm.rot_y %[[REG0_0]], %[[C1_ARG]], %[[C2_ARG]]
  // CHECK-NEXT: %[[REG0_1:.*]] = netqasm.measure %[[REG0_0]] : i1
  // CHECK-NEXT: netqasm.return %[[REG0_1]] : i1

  // CHECK: qoalahost.main_func @test_local_quantum_program()
  qmem.func @test_local_quantum_program() {
    // CHECK: qoalahost.blk_meta {block_id = "[[BLOCK_0:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: %[[C1_i32:.+]] = arith.constant 1 : i32
    // CHECK-NEXT: %[[C2_i32:.+]] = arith.constant 2 : i32
    // CHECK-NEXT: %[[C3_i32:.+]] = arith.constant 3 : i32

    // CHECK: qoalahost.blk_meta {block_id = "[[BLOCK_1:.*]]", deadlines = {}, dependencies = ["[[BLOCK_0]]"], predecessors = [], prev_comm = "", prev_ent = ""}
    %0 = qmem.qalloc : i32
    qmem.init %0

    // This first test case assumes that *ALL* data dependencies are declared _BEFORE_ the quantum calls
    // We don't check the existence of the constants; they are eliminated by the translation process
    %cst_0 = arith.constant 2.356194 : f32
    %cst_1 = arith.constant 0.785398 : f32

    // CHECK: %[[REG_MAIN2:.*]] = qoalahost.call @[[WRAPPER0]](%[[C3_i32]], %[[C2_i32]], %[[C1_i32]]) : (i32, i32, i32) -> i1
    qmem.rot_x %0, %cst_0
    qmem.rot_y %0, %cst_1
    %2 = qmem.measure %0 : i1

    // CHECK: ^[[BLK_1:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_2:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.return
    qmem.return
  }
}