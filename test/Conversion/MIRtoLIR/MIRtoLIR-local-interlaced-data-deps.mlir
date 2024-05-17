// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

// CHECK: module
module {
  // CHECK-LABEL: netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)

  // CHECK: netqasm.local_routine @[[WRAPPER0:.*]]() -> i32
  // CHECK-NEXT: %[[REG0_0:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.init %[[REG0_0]]
  // CHECK-NEXT: netqasm.return %[[REG0_0]] : i32

  // CHECK: netqasm.local_routine @[[WRAPPER1:.*]]() -> i32
  // CHECK-NEXT: %[[REG0_1:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.init %[[REG0_1]]
  // CHECK-NEXT: netqasm.return %[[REG0_1]] : i32

  // CHECK: netqasm.local_routine @[[WRAPPER2:.*]](%[[ARG0_2:.*]]: i32, %[[ARG1_2:.*]]: i32, %[[ARG2_2:.*]]: i32, %[[ARG3_2:.*]]: i32, %[[ARG4_2:.*]]: i32, %[[ARG5_2:.*]]: i32, %[[ARG6_2:.*]]: i32, %[[ARG7_2:.*]]: i32, %[[ARG8_2:.*]]: i32, %[[ARG9_2:.*]]: i32) -> (i1, i1)
  // CHECK-NEXT: netqasm.rot_x %[[ARG0_2]], %[[ARG1_2]], %[[ARG2_2]]
  // CHECK-NEXT: netqasm.rot_y %[[ARG0_2]], %[[ARG3_2]], %[[ARG4_2]]
  // CHECK-NEXT: netqasm.rot_z %[[ARG0_2]], %[[ARG5_2]], %[[ARG6_2]]
  // CHECK-NEXT: netqasm.hadamard %[[ARG7_2]]
  // CHECK-NEXT: netqasm.cnot %[[ARG0_2]], %[[ARG7_2]]
  // CHECK-NEXT: netqasm.cz %[[ARG0_2]], %[[ARG7_2]]
  // CHECK-NEXT: netqasm.crot_x %[[ARG0_2]], %[[ARG7_2]], %[[ARG8_2]], %[[ARG9_2]]
  // CHECK-NEXT: %[[REG0_2:.*]] = netqasm.measure %[[ARG0_2]] : i1
  // CHECK-NEXT: %[[REG1_2:.*]] = netqasm.measure %[[ARG7_2]] : i1
  // CHECK-NEXT: netqasm.return %[[REG0_2]], %[[REG1_2]] : i1, i1

  // CHECK: qoalahost.main_func @test_local_quantum_program()
  qmem.func @test_local_quantum_program() {
    // CHECK: %[[REG_MAIN0:.*]] = qoalahost.call @[[WRAPPER0]]() : () -> i32
    %0 = qmem.qalloc : i32
    qmem.init %0

    // CHECK: %[[REG_MAIN1:.*]] = qoalahost.call @[[WRAPPER1]]() : () -> i32
    %1 = qmem.qalloc : i32
    qmem.init %1

    // This first test case assumes that *ALL* data dependencies are declared _BEFORE_ the quantum calls
    // CHECK: %[[CST_0:.*]] = arith.constant
    // CHECK-NEXT: %[[REG_MAIN2:.*]]:2 = qoalahost.call @__qoala_convert_float_angle(%[[CST_0]]) : (f32) -> (i32, i32)
    %cst_0 = arith.constant 0.0306796152 : f32

    qmem.rot_x %0, %cst_0

    // CHECK: %[[CST_1:.*]] = arith.constant
    // CHECK-NEXT: %[[REG_MAIN3:.*]]:2 = qoalahost.call @__qoala_convert_float_angle(%[[CST_1]]) : (f32) -> (i32, i32)
    %cst_1 = arith.constant 1.050000e+01 : f32
    qmem.rot_y %0, %cst_1

    // CHECK: %[[CST:.*]] = arith.constant
    // CHECK-NEXT: %[[REG_MAIN4:.*]]:2 = qoalahost.call @__qoala_convert_float_angle(%[[CST]]) : (f32) -> (i32, i32)
    %cst = arith.constant 2.120000e+01 : f32
    qmem.rot_z %0, %cst
    qmem.hadamard %1
    qmem.cnot %0, %1
    qmem.cz %0, %1

    // CHECK: %[[CST_2:.*]] = arith.constant
    // CHECK-NEXT: %[[REG_MAIN5:.*]]:2 = qoalahost.call @__qoala_convert_float_angle(%[[CST_2]]) : (f32) -> (i32, i32)
    %cst_2 = arith.constant 2.710000e+00 : f32
    qmem.crot_x %0, %1, %cst_2
    %2 = qmem.measure %0 : i1
    %3 = qmem.measure %1 : i1
    // CHECK-NEXT: %[[REG_MAIN6:.*]]:2 = qoalahost.call @[[WRAPPER2]](%[[REG_MAIN0]], %[[REG_MAIN2]]#0, %[[REG_MAIN2]]#1, %[[REG_MAIN3]]#0, %[[REG_MAIN3]]#1, %[[REG_MAIN4]]#0, %[[REG_MAIN4]]#1, %[[REG_MAIN1]], %[[REG_MAIN5]]#0, %[[REG_MAIN5]]#1) : (i32, i32, i32, i32, i32, i32, i32, i32, i32, i32) -> (i1, i1)

    // CHECK: qoalahost.return
    qmem.return
  }
}