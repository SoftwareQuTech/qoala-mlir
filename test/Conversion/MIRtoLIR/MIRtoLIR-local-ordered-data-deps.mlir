// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

// TODO - converting this code generates invalid assembly: error: operand #3 does not dominate this use "qmem.rot_x %0, %cst_0"

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
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER2:.*]]() -> (i1, i1)
  // CHECK-NEXT: netqasm.rot_x %[[ARG0_4]], %[[ARG1_4]], %[[ARG2_4]]
  // CHECK-NEXT: netqasm.rot_y %[[ARG0_5]], %[[ARG1_5]], %[[ARG2_5]]
  // CHECK-NEXT: netqasm.rot_z %[[ARG0_6]], %[[ARG1_6]], %[[ARG2_6]]
  // CHECK-NEXT: netqasm.hadamard %[[ARG0_7]]
  // CHECK-NEXT: netqasm.cnot %[[ARG0_8]], %[[ARG1_8]]
  // CHECK-NEXT: netqasm.cz %[[ARG0_9]], %[[ARG1_9]]
  // CHECK-NEXT: netqasm.crot_x %[[ARG0_10]], %[[ARG1_10]], %[[ARG2_10]], %[[ARG3_10]]
  // CHECK-NEXT: %[[REG0_0:.*]] = netqasm.measure %[[ARG0_11]] : i1
  // CHECK-NEXT: %[[REG0REG0_1:.*]] = netqasm.measure %[[ARG0_12]] : i1
  // CHECK-NEXT: netqasm.return %[[REG0_0]], %[[REG0_1]] : i1, i1

  // CHECK: qoalahost.main_func @test_local_quantum_program()
  qmem.func @test_local_quantum_program() {
    // CHECK: %[[REG_MAIN0:.*]] = qoalahost.call @[[WRAPPER0]]() : () -> i32
    %0 = qmem.qalloc : i32
    qmem.init %0

    // CHECK: %[[REG_MAIN1:.*]] = qoalahost.call @[[WRAPPER1]]() : () -> i32
    %1 = qmem.qalloc : i32
    qmem.init %1

    // This first test case assumes that *ALL* data dependencies are declared _BEFORE_ the quantum calls
    // CHECK: %[[CST:.*]] = arith.constant
    %cst = arith.constant 2.120000e+01 : f32
    // CHECK: %[[CST_0:.*]] = arith.constant
    %cst_0 = arith.constant 0.0306796152 : f32
    // CHECK: %[[CST_1:.*]] = arith.constant
    %cst_1 = arith.constant 1.050000e+01 : f32
    // CHECK: %[[CST_2:.*]] = arith.constant
    %cst_2 = arith.constant 2.710000e+00 : f32

    // CHECK: %[[REG_MAIN2:.*]]:2 = qoalahost.call @__qoala_convert_float_angle(%[[CST_0]]) : (f32) -> (i32, i32)
    // CHECK-NEXT: qoalahost.call %[[REG_MAIN3]]:2 = @[[WRAPPER2]](%[[REG_MAIN0]], %[[REG_MAIN2]]#0, %[[REG_MAIN2]]#1) : (i32, i32, i32) -> ()
    qmem.rot_x %0, %cst_0
    qmem.rot_y %0, %cst_1
    qmem.rot_z %0, %cst
    qmem.hadamard %1
    qmem.cnot %0, %1
    qmem.cz %0, %1
    qmem.crot_x %0, %1, %cst_2
    %2 = qmem.measure %0 : i1
    %3 = qmem.measure %1 : i1

    // CHECK: qoalahost.return
    qmem.return
  }
}