// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

// CHECK: module
module {
  // CHECK-LABEL: netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)

  // CHECK: netqasm.local_routine @[[WRAPPER0:.*]](%[[C0_ARG:.+]]: i32, %[[C1_ARG:.+]]: i32, %[[C2_ARG:.+]]: i32, %[[C3_ARG:.+]]: i32, %[[C22_ARG:.+]]: i32, %[[C25_ARG:.+]]: i32, %[[C29_ARG:.+]]: i32, %[[C1000000_ARG:.+]]: i32, %[[C1750000_ARG:.+]]: i32, %[[C7_ARG:.+]]: i32, %[[C4_ARG:.+]]: i32, %[[C67_ARG:.+]]: i32, %[[C5_ARG:.+]]: i32, %[[C169_ARG:.+]]: i32, %[[C12_ARG:.+]]: i32) -> i1
  // CHECK-NEXT: %[[REG0:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.init %[[REG0]]
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], %[[C0_ARG]], %[[C0_ARG]]
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], %[[C1_ARG]], %[[C0_ARG]]
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], %[[C1_ARG]], %[[C1_ARG]]
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], %[[C2_ARG]], %[[C0_ARG]]
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], %[[C1_ARG]], %[[C2_ARG]]
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], %[[C1_ARG]], %[[C3_ARG]]
  // Small numbers
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], %[[C1_ARG]], %[[C22_ARG]]
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], %[[C1_ARG]], %[[C25_ARG]]
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], %[[C1_ARG]], %[[C29_ARG]]
  // Big numbers
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], %[[C1000000_ARG]], %[[C0_ARG]]
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], %[[C1750000_ARG]], %[[C0_ARG]]
  // Primes
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], %[[C3_ARG]], %[[C2_ARG]]
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], %[[C7_ARG]], %[[C4_ARG]]
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], %[[C67_ARG]], %[[C5_ARG]]
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], %[[C169_ARG]], %[[C12_ARG]]

  // CHECK-NEXT: %[[REG1:.*]] = netqasm.measure %[[REG0]] : i1
  // CHECK-NEXT: netqasm.return %[[REG1]] : i1

  // CHECK: qoalahost.main_func @test_local_quantum_program()
  // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
  qmem.func @test_local_quantum_program() {
    %0 = qmem.qalloc : i32
    qmem.init %0

    // Some "basic" test cases for exploring the rotations
    // 0 = 0\pi/2^0 -> (0, 0)
    %cst = arith.constant 0.000000 : f32
    // 3.141592 = 1\pi/2^0 -> (1, 0)
    %cst_0 = arith.constant 3.141592 : f32
    // 1.570796 = 1\pi/2^1 -> (1, 1)
    %cst_1 = arith.constant 1.570796 : f32
    // 6.283184 = 2\pi/2^0 -> (2, 0)
    %cst_2 = arith.constant 6.283184 : f32
    // 0.785398 = 1\pi/2^2 -> (1, 2)
    %cst_3 = arith.constant 0.785398 : f32
    // 0.392699 = 1\pi/2^3 -> (1, 3)
    %cst_4 = arith.constant 0.392699 : f32

    // Some small numbers:
    // 0.00000074 = 1\pi/2^22 -> (1, 22)
    %cst_5 = arith.constant 0.00000074 : f32
    // 0.0000000936 = 1\pi/2^25
    %cst_6 = arith.constant 0.0000000936 : f32
    // 0.00000000585 = 1\pi/2^29
    %cst_7 = arith.constant 0.00000000585 : f32

    // "big" numbers:
    // 3141592.0 = 10^6\pi/2^0 -> (1000000, 0)
    %cst_8 = arith.constant 3141592.0 : f32
    // 5497786.0 = 7*10^6\pi/2^2 -> 3.5*10^6\pi/2^1 -> 1.75*10^6\pi/2^0 -> (1750000, 0)
    %cst_9 = arith.constant 5497786.0 : f32

    // Some relative prime integers to find
    // 2.356194 = 3\pi/2^2 -> (3, 2)
    %cst_a = arith.constant 2.356194 : f32
    // 1.3744465 = 7\pi/2^4 -> (7, 4)
    %cst_b = arith.constant 1.3744465 : f32
    // 6.57770825 = 67\pi/2^5 -> (67, 5)
    %cst_c = arith.constant 6.57770825 : f32
    // Hard to find:  169\pi/2^12 = 0.12962135 -> (169, 12)
    %cst_d = arith.constant 1.3e-01 : f32

    // Constants introduced by the compilation (folded if the same constant inserted more than once)
    // CHECK: %[[C12:.+]] = arith.constant 12 : i32
    // CHECK-NEXT: %[[C169:.+]] = arith.constant 169 : i32
    // CHECK-NEXT: %[[C5:.+]] = arith.constant 5 : i32
    // CHECK-NEXT: %[[C67:.+]] = arith.constant 67 : i32
    // CHECK-NEXT: %[[C4:.+]] = arith.constant 4 : i32
    // CHECK-NEXT: %[[C7:.+]] = arith.constant 7 : i32
    // CHECK-NEXT: %[[C1750000:.+]] = arith.constant 1750000 : i32
    // CHECK-NEXT: %[[C1000000:.+]] = arith.constant 1000000 : i32
    // CHECK-NEXT: %[[C29:.+]] = arith.constant 29 : i32
    // CHECK-NEXT: %[[C25:.+]] = arith.constant 25 : i32
    // CHECK-NEXT: %[[C22:.+]] = arith.constant 22 : i32
    // CHECK-NEXT: %[[C3:.+]] = arith.constant 3 : i32
    // CHECK-NEXT: %[[C2:.+]] = arith.constant 2 : i32
    // CHECK-NEXT: %[[C1:.+]] = arith.constant 1 : i32
    // CHECK-NEXT: %[[C0:.+]] = arith.constant 0 : i32

    // CHECK: %[[REG_CALL:.*]] = qoalahost.call @[[WRAPPER0]](%[[C0]], %[[C1]], %[[C2]], %[[C3]], %[[C22]], %[[C25]], %[[C29]], %[[C1000000]], %[[C1750000]], %[[C7]], %[[C4]], %[[C67]], %[[C5]], %[[C169]], %[[C12]]) : (i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32) -> i1
    qmem.rot_x %0, %cst
    qmem.rot_x %0, %cst_0
    qmem.rot_x %0, %cst_1
    qmem.rot_x %0, %cst_2
    qmem.rot_x %0, %cst_3
    qmem.rot_x %0, %cst_4
    qmem.rot_x %0, %cst_5
    qmem.rot_x %0, %cst_6
    qmem.rot_x %0, %cst_7
    qmem.rot_x %0, %cst_8
    qmem.rot_x %0, %cst_9
    qmem.rot_x %0, %cst_a
    qmem.rot_x %0, %cst_b
    qmem.rot_x %0, %cst_c
    qmem.rot_x %0, %cst_d
    %1 = qmem.measure %0 : i1

    // CHECK-NEXT: ^[[BLOCK_1:.*]]:
    // CHECK: qoalahost.return
    qmem.return
  }
}
