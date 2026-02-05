// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

// CHECK: module
module {
  // CHECK-LABEL: netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)


  // CHECK: netqasm.local_routine @[[WRAPPER0:.*]]() -> i32
  // CHECK-NEXT: %[[REG0_0:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.init %[[REG0_0]]
  // CHECK-NEXT: netqasm.return %[[REG0_0]] : i32

  // CHECK: netqasm.local_routine @[[WRAPPER1:.*]]() -> i32
  // CHECK-NEXT: %[[REG1_0:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.init %[[REG1_0]]
  // CHECK-NEXT: netqasm.return %[[REG1_0]] : i32

  // CHECK: netqasm.local_routine @[[WRAPPER2:.*]](%[[QUBIT_A:.*]]: i32, %[[C3_ARG:.+]]: i32, %[[C2_ARG:.+]]: i32, %[[C1_ARG:.+]]: i32, %[[C4_ARG:.+]]: i32)
  // CHECK-NEXT: netqasm.rot_x %[[QUBIT_A]], %[[C3_ARG]], %[[C2_ARG]]
  // CHECK-NEXT: netqasm.rot_y %[[QUBIT_A]], %[[C1_ARG]], %[[C2_ARG]]
  // CHECK-NEXT: netqasm.rot_z %[[QUBIT_A]], %[[C3_ARG]], %[[C4_ARG]]
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER3:.*]](%[[QUBIT_B:.*]]: i32)
  // CHECK-NEXT: netqasm.hadamard %[[QUBIT_B]]
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER4:.*]](%[[QUBIT_C:.*]]: i32, %[[QUBIT_D:.*]]: i32, %[[C1_ARG_B:.+]]: i32)
  // CHECK-NEXT: netqasm.cnot %[[QUBIT_C]], %[[QUBIT_D]]
  // CHECK-NEXT: netqasm.cz %[[QUBIT_C]], %[[QUBIT_D]]
  // CHECK-NEXT: netqasm.crot_x %[[QUBIT_C]], %[[QUBIT_D]], %[[C1_ARG_B]], %[[C1_ARG_B]]
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER5:.*]](%[[QUBIT_E:.*]]: i32) -> i1
  // CHECK-NEXT: netqasm.x %[[QUBIT_E]]
  // CHECK-NEXT: netqasm.y %[[QUBIT_E]]
  // CHECK-NEXT: netqasm.z %[[QUBIT_E]]
  // CHECK-NEXT: %[[REG2_0:.*]] = netqasm.measure %[[QUBIT_E]] : i1
  // CHECK-NEXT: netqasm.return %[[REG2_0]] : i1

  // CHECK: netqasm.local_routine @[[WRAPPER6:.*]](%[[QUBIT_F:.*]]: i32) -> i1
  // CHECK-NEXT: %[[REG3_0:.*]] = netqasm.measure %[[QUBIT_F]] : i1
  // CHECK-NEXT: netqasm.return %[[REG3_0]] : i1

  // CHECK: qoalahost.main_func @test_local_quantum_program()
  qmem.func @test_local_quantum_program() {
    // CHECK: qoalahost.blk_meta {block_id = "[[BLOCK_0:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: %[[C4_i32:.+]] = arith.constant 4 : i32
    // CHECK-NEXT: %[[C1_i32:.+]] = arith.constant 1 : i32
    // CHECK-NEXT: %[[C2_i32:.+]] = arith.constant 2 : i32
    // CHECK-NEXT: %[[C3_i32:.+]] = arith.constant 3 : i32

    // CHECK: qoalahost.blk_meta {block_id = "[[BLOCK_1:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: %[[QUBIT_0:.*]] = qoalahost.call @[[WRAPPER0]]() : () -> i32
    %0 = qmem.qalloc : i32
    qmem.init %0

    // CHECK: ^[[BLK_1:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_2:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: %[[QUBIT_1:.*]] = qoalahost.call @[[WRAPPER1]]() : () -> i32
    %1 = qmem.qalloc : i32
    qmem.init %1

    // This first test case assumes that *ALL* data dependencies are declared _BEFORE_ the quantum calls
    // We don't check the existence of the constants; they are eliminated by the translation process

    // CHECK: ^[[BLK_2:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_3:.*]]", deadlines = {}, dependencies = ["[[BLOCK_0]]", "[[BLOCK_1]]"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER2]](%[[QUBIT_0]], %[[C3_i32]], %[[C2_i32]], %[[C1_i32]], %[[C4_i32]]) : (i32, i32, i32, i32, i32) -> ()
    %cst_0 = arith.constant 2.356194 : f32

    qmem.rot_x %0, %cst_0

    %cst_1 = arith.constant 0.785398 : f32
    qmem.rot_y %0, %cst_1

    %cst = arith.constant 0.5890485 : f32
    qmem.rot_z %0, %cst

    // CHECK: ^[[BLK_3:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_4:.*]]", deadlines = {}, dependencies = ["[[BLOCK_2]]"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER3]](%[[QUBIT_1]]) : (i32) -> ()
    qmem.hadamard %1

    // CHECK: ^[[BLK_4:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_5:.*]]", deadlines = {}, dependencies = ["[[BLOCK_3]]", "[[BLOCK_4]]"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER4]](%[[QUBIT_0]], %[[QUBIT_1]], %[[C1_i32]]) : (i32, i32, i32) -> ()
    qmem.cnot %0, %1
    qmem.cz %0, %1

    %cst_2 = arith.constant 1.570796 : f32
    qmem.crot_x %0, %1, %cst_2

    qmem.x %0
    qmem.y %0
    qmem.z %0

    // CHECK: ^[[BLK_5:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_6:.*]]", deadlines = {}, dependencies = ["[[BLOCK_5]]"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: %[[UNUSED_REG_0:.*]] = qoalahost.call @[[WRAPPER5]](%[[QUBIT_0]]) : (i32) -> i1
    %2 = qmem.measure %0 : i1

    // CHECK: ^[[BLK_6:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_7:.*]]", deadlines = {}, dependencies = ["[[BLOCK_5]]"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: %[[UNUSED_REG_1:.*]] = qoalahost.call @[[WRAPPER6]](%[[QUBIT_1]]) : (i32) -> i1
    %3 = qmem.measure %1 : i1

    // CHECK: ^[[BLK_7:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_8:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.return
    qmem.return
  }
}