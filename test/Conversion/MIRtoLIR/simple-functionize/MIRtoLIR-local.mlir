// RUN: qoala-opt %s --lower-qoala-mir-to-lir=use-simple-functionize | FileCheck %s

// CHECK: module
module {
  // CHECK-LABEL: netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)

  // CHECK: netqasm.local_routine @[[WRAPPER0:.*]]() -> i32
  // CHECK-NEXT: %[[REG0_0:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.init %[[REG0_0]]
  // CHECK-NEXT: netqasm.return %[[REG0_0]] : i32

  // CHECK: netqasm.local_routine @[[WRAPPER1:.*]]() -> i32
  // CHECK-NEXT: %[[REG0_2:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.init %[[REG0_2]]
  // CHECK-NEXT: netqasm.return %[[REG0_2]] : i32

  // CHECK: netqasm.local_routine @[[WRAPPER2:.*]](%[[ARG0_4:.*]]: i32, %[[C3_ARG:.+]]: i32, %[[C2_ARG:.+]]: i32)
  // CHECK-NEXT: netqasm.rot_x %[[ARG0_4]], %[[C3_ARG]], %[[C2_ARG]]
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER3:.*]](%[[ARG0_5:.*]]: i32, %[[C1_ARG:.+]]: i32, %[[C2_ARG:.+]]: i32)
  // CHECK-NEXT: netqasm.rot_y %[[ARG0_5]], %[[C1_ARG]], %[[C2_ARG]]
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER4:.*]](%[[ARG0_6:.*]]: i32, %[[C3_ARG:.+]]: i32, %[[C4_ARG:.+]]: i32)
  // CHECK-NEXT: netqasm.rot_z %[[ARG0_6]], %[[C3_ARG]], %[[C4_ARG]]
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER5:.*]](%[[ARG0_7:.*]]: i32)
  // CHECK-NEXT: netqasm.hadamard %[[ARG0_7]]
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER6:.*]](%[[ARG0_8:.*]]: i32, %[[ARG1_8:.*]]: i32)
  // CHECK-NEXT: netqasm.cnot %[[ARG0_8]], %[[ARG1_8]]
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER7:.*]](%[[ARG0_9:.*]]: i32, %[[ARG1_9:.*]]: i32)
  // CHECK-NEXT: netqasm.cz %[[ARG0_9]], %[[ARG1_9]]
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER8:.*]](%[[ARG0_10:.*]]: i32, %[[ARG1_10:.*]]: i32, %[[C1_ARG:.+]]: i32)
  // CHECK-NEXT: netqasm.crot_x %[[ARG0_10]], %[[ARG1_10]], %[[C1_ARG]], %[[C1_ARG]]
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER9:.*]](%[[ARG0_11:.*]]: i32)
  // CHECK-NEXT: netqasm.x %[[ARG0_11]]
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER10:.*]](%[[ARG0_12:.*]]: i32)
  // CHECK-NEXT: netqasm.y %[[ARG0_12]]
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER11:.*]](%[[ARG0_13:.*]]: i32)
  // CHECK-NEXT: netqasm.z %[[ARG0_13]]
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER12:.*]](%[[ARG0_14:.*]]: i32) -> i1
  // CHECK-NEXT: %[[REG0_11:.*]] = netqasm.measure %[[ARG0_14]] : i1
  // CHECK-NEXT: netqasm.return %[[REG0_11]] : i1

  // CHECK: netqasm.local_routine @[[WRAPPER13:.*]](%[[ARG0_15:.*]]: i32) -> i1
  // CHECK-NEXT: %[[REG0_12:.*]] = netqasm.measure %[[ARG0_15]] : i1
  // CHECK-NEXT: netqasm.return %[[REG0_12]] : i1

  // CHECK: qoalahost.main_func @test_local_quantum_program()
  qmem.func @test_local_quantum_program() {
    // CHECK: qoalahost.blk_meta {block_id = "[[BLOCK_0:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: %[[C4_i32:.+]] = arith.constant 4 : i32
    // CHECK-NEXT: %[[C1_i32:.+]] = arith.constant 1 : i32
    // CHECK-NEXT: %[[C2_i32:.+]] = arith.constant 2 : i32
    // CHECK-NEXT: %[[C3_i32:.+]] = arith.constant 3 : i32

    // CHECK: qoalahost.blk_meta {block_id = "[[BLOCK_1:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: %[[REG_MAIN0:.*]] = qoalahost.call @[[WRAPPER0]]() : () -> i32
    %0 = qmem.qalloc : i32
    qmem.init %0

    // CHECK: ^[[BLK_1:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_2:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: %[[REG_MAIN1:.*]] = qoalahost.call @[[WRAPPER1]]() : () -> i32
    %1 = qmem.qalloc : i32
    qmem.init %1

    // We don't check the existence of the constants; they are eliminated by the translation process
    %cst = arith.constant 0.5890485 : f32
    %cst_0 = arith.constant 2.356194 : f32

    // CHECK: ^[[BLK_2:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_3:.*]]", deadlines = {}, dependencies = ["[[BLOCK_0]]", "[[BLOCK_1]]"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER2]](%[[REG_MAIN0]], %[[C3_i32]], %[[C2_i32]]) : (i32, i32, i32) -> ()
    qmem.rot_x %0, %cst_0

    %cst_1 = arith.constant 0.785398 : f32

    // CHECK: ^[[BLK_3:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_4:.*]]", deadlines = {}, dependencies = ["[[BLOCK_3]]"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER3]](%[[REG_MAIN0]], %[[C1_i32]], %[[C2_i32]]) : (i32, i32, i32) -> ()
    qmem.rot_y %0, %cst_1

    // CHECK: ^[[BLK_4:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_5:.*]]", deadlines = {}, dependencies = ["[[BLOCK_4]]"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER4]](%[[REG_MAIN0]], %[[C3_i32]], %[[C4_i32]]) : (i32, i32, i32) -> ()
    qmem.rot_z %0, %cst

    // CHECK: ^[[BLK_5:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_6:.*]]", deadlines = {}, dependencies = ["[[BLOCK_2]]"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER5]](%[[REG_MAIN1]]) : (i32) -> ()
    qmem.hadamard %1

    // CHECK: ^[[BLK_6:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_7:.*]]", deadlines = {}, dependencies = ["[[BLOCK_5]]", "[[BLOCK_6]]"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER6]](%[[REG_MAIN0]], %[[REG_MAIN1]]) : (i32, i32) -> ()
    qmem.cnot %0, %1

    %cst_2 = arith.constant 1.570796 : f32

    // CHECK: ^[[BLK_7:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_8:.*]]", deadlines = {}, dependencies = ["[[BLOCK_7]]"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER7]](%[[REG_MAIN0]], %[[REG_MAIN1]]) : (i32, i32) -> ()
    qmem.cz %0, %1

    // CHECK: ^[[BLK_8:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_9:.*]]", deadlines = {}, dependencies = ["[[BLOCK_8]]"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER8]](%[[REG_MAIN0]], %[[REG_MAIN1]], %[[C1_i32]]) : (i32, i32, i32) -> ()
    qmem.crot_x %0, %1, %cst_2

    // CHECK: ^[[BLK_9:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_10:.*]]", deadlines = {}, dependencies = ["[[BLOCK_9]]"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER9]](%[[REG_MAIN0]]) : (i32) -> ()
    qmem.x %0

    // CHECK: ^[[BLK_10:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_11:.*]]", deadlines = {}, dependencies = ["[[BLOCK_10]]"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER10]](%[[REG_MAIN0]]) : (i32) -> ()
    qmem.y %0

    // CHECK: ^[[BLK_11:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_12:.*]]", deadlines = {}, dependencies = ["[[BLOCK_11]]"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER11]](%[[REG_MAIN0]]) : (i32) -> ()
    qmem.z %0

    // CHECK: ^[[BLK_12:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_13:.*]]", deadlines = {}, dependencies = ["[[BLOCK_12]]"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: %[[REG_MAIN6:.*]] = qoalahost.call @[[WRAPPER12]](%[[REG_MAIN0]]) : (i32) -> i1
    %2 = qmem.measure %0 : i1

    // CHECK: ^[[BLK_13:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_14:.*]]", deadlines = {}, dependencies = ["[[BLOCK_9]]"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: %[[REG_MAIN7:.*]] = qoalahost.call @[[WRAPPER13]](%[[REG_MAIN1]]) : (i32) -> i1
    %3 = qmem.measure %1 : i1

    // CHECK: ^[[BLK_14:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_15:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.return
    qmem.return
  }
}