// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

// CHECK: module
module {
  // CHECK-LABEL: netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)

  // CHECK: netqasm.local_routine @[[WRAPPER0:.*]]() -> i32
  // CHECK-NEXT: %[[REG0_0:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.init %[[REG0_0]]
  // CHECK-NEXT: netqasm.return %[[REG0_0]] : i32

  // CHECK: netqasm.local_routine @[[WRAPPER1:.*]](%[[ARG0_2:.*]]: i32, %[[ARG1_2:.*]]: i32, %[[ARG2_2:.*]]: i32, %[[ARG3_2:.*]]: i32, %[[ARG4_2:.*]]: i32) -> i1
  // CHECK-NEXT: netqasm.rot_x %[[ARG0_2]], %[[ARG1_2]], %[[ARG2_2]]
  // CHECK-NEXT: netqasm.rot_y %[[ARG0_2]], %[[ARG3_2]], %[[ARG4_2]]
  // CHECK-NEXT: %[[REG0_0:.*]] = netqasm.measure %[[ARG0_2]] : i1
  // CHECK-NEXT: netqasm.return %[[REG0_0]] : i1

  // CHECK: qoalahost.main_func @test_local_quantum_program()
  qmem.func @test_local_quantum_program() {
    // CHECK: %[[REG_MAIN0:.*]] = qoalahost.call @[[WRAPPER0]]() : () -> i32
    %0 = qmem.qalloc : i32
    qmem.init %0

    // This first test case assumes that *ALL* data dependencies are declared _BEFORE_ the quantum calls
    // CHECK: %[[CST:.*]] = arith.constant
    %cst = arith.constant 2.120000e+01 : f32
    // CHECK: %[[CST_0:.*]] = arith.constant
    %cst_0 = arith.constant 0.0306796152 : f32
    // CHECK: %[[CST_1:.*]] = arith.constant
    %cst_1 = arith.constant 1.050000e+01 : f32
    // CHECK: %[[CST_2:.*]] = arith.constant
    %cst_2 = arith.constant 2.710000e+00 : f32

    // CHECK: %[[REG_MAIN1:.*]]:2 = qoalahost.call @__qoala_convert_float_angle(%[[CST_0]]) : (f32) -> (i32, i32)
    // CHECK: %[[REG_MAIN2:.*]]:2 = qoalahost.call @__qoala_convert_float_angle(%[[CST_1]]) : (f32) -> (i32, i32)
    // CHECK-NEXT: %[[REG_MAIN3:.*]] = qoalahost.call @[[WRAPPER1]](%[[REG_MAIN0]], %[[REG_MAIN1]]#0, %[[REG_MAIN1]]#1, %[[REG_MAIN2]]#0, %[[REG_MAIN2]]#1) : (i32, i32, i32, i32, i32) -> i1
    qmem.rot_x %0, %cst_0
    qmem.rot_y %0, %cst_1
    %2 = qmem.measure %0 : i1

    // CHECK: qoalahost.return
    qmem.return
  }
}