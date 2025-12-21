// RUN: qoala-opt %s --lower-qoala-mir-to-lir=max-ops-per-group=3 | FileCheck %s

// CHECK: module
module {
  // CHECK-LABEL: netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)

  // CHECK: netqasm.local_routine @[[WRAPPER0:.*]]() -> i32
  // CHECK-NEXT: %[[REG0_0:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.init %[[REG0_0]]
  // CHECK-NEXT: netqasm.rot_x %[[REG0_0]] (3 : ui32, 2 : ui32)
  // CHECK-NEXT: netqasm.return %[[REG0_0]] : i32

  // CHECK: netqasm.local_routine @[[WRAPPER1:.*]](%[[QUBIT_A:.*]]: i32)
  // CHECK-NEXT: netqasm.rot_y %[[QUBIT_A]] (1 : ui32, 2 : ui32)
  // CHECK-NEXT: netqasm.rot_z %[[QUBIT_A]] (3 : ui32, 4 : ui32)
  // CHECK-NEXT: netqasm.hadamard %[[QUBIT_A]]
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER2:.*]](%[[QUBIT_B:.*]]: i32) -> i1
  // CHECK-NEXT: netqasm.rot_x %[[QUBIT_B]] (3 : ui32, 2 : ui32)
  // CHECK-NEXT: netqasm.rot_y %[[QUBIT_B]] (1 : ui32, 2 : ui32)
  // CHECK-NEXT: %[[REG3_0:.*]] = netqasm.measure %[[QUBIT_B]] : i1
  // CHECK-NEXT: netqasm.return %[[REG3_0]] : i1

  // CHECK: qoalahost.main_func @test_local_quantum_program()
  qmem.func @test_local_quantum_program() {
    // CHECK: qoalahost.blk_meta {block_id = "[[BLOCK_0:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: %[[QUBIT_0:.*]] = qoalahost.call @[[WRAPPER0]]() : () -> i32
    %0 = qmem.qalloc : i32
    qmem.init %0

    %cst = arith.constant 0.5890485 : f32
    %cst_0 = arith.constant 2.356194 : f32
    %cst_1 = arith.constant 0.785398 : f32

    qmem.rot_x %0, %cst_0

    // CHECK: ^[[BLK_1:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_1:.*]]", deadlines = {}, dependencies = ["[[BLOCK_0]]"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER1]](%[[QUBIT_0]]) : (i32) -> ()
    qmem.rot_y %0, %cst_1
    qmem.rot_z %0, %cst
    qmem.hadamard %0

    // CHECK: ^[[BLK_2:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_2:.*]]", deadlines = {}, dependencies = ["[[BLOCK_1]]"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: %[[UNUSED_REG_0:.*]] = qoalahost.call @[[WRAPPER2]](%[[QUBIT_0]]) : (i32) -> i1
    qmem.rot_x %0, %cst_0
    qmem.rot_y %0, %cst_1
    %2 = qmem.measure %0 : i1

    // CHECK: ^[[BLK_3:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_3:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.return
    qmem.return
  }
}