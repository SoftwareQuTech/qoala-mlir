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

  // CHECK: netqasm.local_routine @[[WRAPPER2:.*]](%[[ARG0_4:.*]]: i32)
  // CHECK-NEXT: netqasm.rot_x %[[ARG0_4]] (3 : ui32, 2 : ui32)
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER3:.*]](%[[ARG0_5:.*]]: i32)
  // CHECK-NEXT: netqasm.rot_y %[[ARG0_5]] (1 : ui32, 2 : ui32)
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER4:.*]](%[[ARG0_6:.*]]: i32)
  // CHECK-NEXT: netqasm.rot_z %[[ARG0_6]] (3 : ui32, 4 : ui32)
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

  // CHECK: netqasm.local_routine @[[WRAPPER8:.*]](%[[ARG0_10:.*]]: i32, %[[ARG1_10:.*]]: i32)
  // CHECK-NEXT: netqasm.crot_x %[[ARG0_10]], %[[ARG1_10]] (1 : ui32, 1 : ui32)
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER9:.*]](%[[ARG0_11:.*]]: i32) -> i1
  // CHECK-NEXT: %[[REG0_11:.*]] = netqasm.measure %[[ARG0_11]] : i1
  // CHECK-NEXT: netqasm.return %[[REG0_11]] : i1

  // CHECK: netqasm.local_routine @[[WRAPPER10:.*]](%[[ARG0_12:.*]]: i32) -> i1
  // CHECK-NEXT: %[[REG0_12:.*]] = netqasm.measure %[[ARG0_12]] : i1
  // CHECK-NEXT: netqasm.return %[[REG0_12]] : i1

  // CHECK: qoalahost.main_func @test_local_quantum_program()
  qmem.func @test_local_quantum_program() {
    // CHECK: qoalahost.blk_meta {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: %[[REG_MAIN0:.*]] = qoalahost.call @[[WRAPPER0]]() : () -> i32
    %0 = qmem.qalloc : i32
    qmem.init %0

    // CHECK: ^[[BLOCK_1:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: %[[REG_MAIN1:.*]] = qoalahost.call @[[WRAPPER1]]() : () -> i32
    %1 = qmem.qalloc : i32
    qmem.init %1

    // We don't check the existence of the constants; they are eliminated by the translation process
    %cst = arith.constant 0.5890485 : f32
    %cst_0 = arith.constant 2.356194 : f32

    // CHECK: ^[[BLOCK_2:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_2", deadlines = {}, dependencies = ["block_0"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER2]](%[[REG_MAIN0]]) : (i32) -> ()
    qmem.rot_x %0, %cst_0

    %cst_1 = arith.constant 0.785398 : f32

    // CHECK: ^[[BLOCK_3:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_3", deadlines = {}, dependencies = ["block_2"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER3]](%[[REG_MAIN0]]) : (i32) -> ()
    qmem.rot_y %0, %cst_1

    // CHECK: ^[[BLOCK_4:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_4", deadlines = {}, dependencies = ["block_3"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER4]](%[[REG_MAIN0]]) : (i32) -> ()
    qmem.rot_z %0, %cst

    // CHECK: ^[[BLOCK_5:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_5", deadlines = {}, dependencies = ["block_1"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER5]](%[[REG_MAIN1]]) : (i32) -> ()
    qmem.hadamard %1

    // CHECK: ^[[BLOCK_6:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_6", deadlines = {}, dependencies = ["block_4", "block_5"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER6]](%[[REG_MAIN0]], %[[REG_MAIN1]]) : (i32, i32) -> ()
    qmem.cnot %0, %1

    %cst_2 = arith.constant 1.570796 : f32

    // CHECK: ^[[BLOCK_7:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_7", deadlines = {}, dependencies = ["block_6"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER7]](%[[REG_MAIN0]], %[[REG_MAIN1]]) : (i32, i32) -> ()
    qmem.cz %0, %1

    // CHECK: ^[[BLOCK_8:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_8", deadlines = {}, dependencies = ["block_7"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER8]](%[[REG_MAIN0]], %[[REG_MAIN1]]) : (i32, i32) -> ()
    qmem.crot_x %0, %1, %cst_2

    // CHECK: ^[[BLOCK_9:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_9", deadlines = {}, dependencies = ["block_8"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: %[[REG_MAIN6:.*]] = qoalahost.call @[[WRAPPER9]](%[[REG_MAIN0]]) : (i32) -> i1
    %2 = qmem.measure %0 : i1

    // CHECK: ^[[BLOCK_10:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_10", deadlines = {}, dependencies = ["block_8"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: %[[REG_MAIN7:.*]] = qoalahost.call @[[WRAPPER10]](%[[REG_MAIN1]]) : (i32) -> i1
    %3 = qmem.measure %1 : i1

    // CHECK: ^[[BLOCK_11:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_11", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.return
    qmem.return
  }
}