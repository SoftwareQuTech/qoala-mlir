// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

// CHECK: module
module {
  // CHECK-LABEL: netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)

  // CHECK: netqasm.local_routine @[[WRAPPER0:.*]]() -> i1
  // CHECK-NEXT: %[[REG0_0:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.init %[[REG0_0]]
  // CHECK-NEXT: netqasm.rot_x %[[REG0_0]] (3 : ui32, 2 : ui32)
  // CHECK-NEXT: netqasm.rot_y %[[REG0_0]] (1 : ui32, 2 : ui32)
  // CHECK-NEXT: %[[REG0_1:.*]] = netqasm.measure %[[REG0_0]] : i1
  // CHECK-NEXT: netqasm.return %[[REG0_1]] : i1

  // CHECK: qoalahost.main_func @test_local_quantum_program()
  qmem.func @test_local_quantum_program() {
    %0 = qmem.qalloc : i32
    qmem.init %0

    // This first test case assumes that *ALL* data dependencies are declared _BEFORE_ the quantum calls
    // CHECK: %[[CST_0:.*]] = arith.constant
    %cst_0 = arith.constant 2.356194 : f32
    // CHECK: %[[CST_1:.*]] = arith.constant
    %cst_1 = arith.constant 0.785398 : f32

    // CHECK: %[[REG_MAIN2:.*]] = qoalahost.call @[[WRAPPER0]]() : () -> i1
    qmem.rot_x %0, %cst_0
    qmem.rot_y %0, %cst_1
    %2 = qmem.measure %0 : i1

    // CHECK: qoalahost.return
    qmem.return
  }
}