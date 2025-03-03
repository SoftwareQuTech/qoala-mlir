// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

// CHECK: module
module {
  // CHECK-LABEL: netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)

  // CHECK: netqasm.local_routine @[[WRAPPER0:.*]]() -> (i1, i1)
  // CHECK-NEXT: %[[REG0_0:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.init %[[REG0_0]]
  // CHECK-NEXT: %[[REG1_0:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.init %[[REG1_0]]
  // CHECK-NEXT: netqasm.rot_x %[[REG0_0]] (3 : ui32, 2 : ui32)
  // CHECK-NEXT: netqasm.rot_y %[[REG0_0]] (1 : ui32, 2 : ui32)
  // CHECK-NEXT: netqasm.rot_z %[[REG0_0]] (3 : ui32, 4 : ui32)
  // CHECK-NEXT: netqasm.hadamard %[[REG1_0]]
  // CHECK-NEXT: netqasm.cnot %[[REG0_0]], %[[REG1_0]]
  // CHECK-NEXT: netqasm.cz %[[REG0_0]], %[[REG1_0]]
  // CHECK-NEXT: netqasm.crot_x %[[REG0_0]], %[[REG1_0]] (1 : ui32, 1 : ui32)
  // CHECK-NEXT: %[[REG2_0:.*]] = netqasm.measure %[[REG0_0]] : i1
  // CHECK-NEXT: %[[REG3_0:.*]] = netqasm.measure %[[REG1_0]] : i1
  // CHECK-NEXT: netqasm.return %[[REG2_0]], %[[REG3_0]] : i1, i1

  // CHECK: qoalahost.main_func @test_local_quantum_program()
  qmem.func @test_local_quantum_program() {
    %0 = qmem.qalloc : i32
    qmem.init %0
    %1 = qmem.qalloc : i32
    qmem.init %1

    // This first test case assumes that *ALL* data dependencies are declared _BEFORE_ the quantum calls
    // CHECK: %[[CST:.*]] = arith.constant
    %cst = arith.constant 0.5890485 : f32
    // CHECK: %[[CST_0:.*]] = arith.constant
    %cst_0 = arith.constant 2.356194 : f32
    // CHECK: %[[CST_1:.*]] = arith.constant
    %cst_1 = arith.constant 0.785398 : f32
    // CHECK: %[[CST_2:.*]] = arith.constant
    %cst_2 = arith.constant 1.570796 : f32

    // CHECK-NEXT: %[[REG_MAIN4:.*]]:2 = qoalahost.call @[[WRAPPER0]]() : () -> (i1, i1)
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