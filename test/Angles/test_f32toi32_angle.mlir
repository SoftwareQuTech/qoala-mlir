// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

// CHECK: module
module {
  // CHECK-LABEL: netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)

  // CHECK: netqasm.local_routine @[[WRAPPER0:.*]]() -> i1
  // CHECK-NEXT: %[[REG0:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.init %[[REG0]]
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], 0, 0
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], 1, 0
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], 1, 1
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], 2, 0
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], 1, 2
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], 1, 3
  // Small numbers
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], 1, 22
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], 1, 25
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], 1, 29
  // Big numbers
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], 1000000, 0
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], 1750000, 0
  // Primes
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], 3, 2
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], 7, 4
  // CHECK-NEXT: netqasm.rot_x %[[REG0]], 67, 5

  // CHECK-NEXT: %[[REG1:.*]] = netqasm.measure %[[REG0]] : i1
  // CHECK-NEXT: netqasm.return %[[REG1]] : i1

  // CHECK: qoalahost.main_func @test_local_quantum_program()
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
    // 6.283184 = 2\pi/2^1 -> (2, 0)
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

    // Non-exact numbers

    // CHECK: %[[REG_CALL:.*]] = qoalahost.call @[[WRAPPER0]]() : () -> i1
    //qmem.rot_x %0, %cst
    //qmem.rot_x %0, %cst_0
    //qmem.rot_x %0, %cst_1
    //qmem.rot_x %0, %cst_2
    //qmem.rot_x %0, %cst_3
    //qmem.rot_x %0, %cst_4
    //qmem.rot_x %0, %cst_5
    //qmem.rot_x %0, %cst_6
    //qmem.rot_x %0, %cst_7
    //qmem.rot_x %0, %cst_8
    //qmem.rot_x %0, %cst_9
    qmem.rot_x %0, %cst_a
    qmem.rot_x %0, %cst_b
    qmem.rot_x %0, %cst_c
    %1 = qmem.measure %0 : i1

    // CHECK: qoalahost.return
    qmem.return
  }
}
