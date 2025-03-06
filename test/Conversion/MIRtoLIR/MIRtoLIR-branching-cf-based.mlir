// UNSUPPORTED: true
// Grouping arith+cf.cond_branch instructions is not supported yet.
// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

// CHECK: module
module {
  // CHECK-LABEL: netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)

  // TODO - Double check the checks of mapped operations for this example
  // CHECK: netqasm.local_routine @[[WRAPPER0:.*]]([[ARG0_0:.*]]: i32) -> i1
  // CHECK-NEXT: %[[REG0_0:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.init %[[REG0_0]]
  // CHECK-NEXT: netqasm.rot_x %[[REG0_0]] (3 : ui32, 2 : ui32)
  // CHECK-NEXT: %[[REG0_1:.*]] = netqasm.measure %[[REG0_0]] : i1
  // CHECK-NEXT: netqasm.return %[[REG0_1]] : i1

  // CHECK: qoalahost.main_func @test_remote_quantum_program()
  qmem.func @test_remote_quantum_program() -> i1 {
    %c0 = arith.constant 0 : i32
    %c1 = arith.constant 1 : i32
    %cst_0 = arith.constant 2.356194 : f32
    %cst_1 = arith.constant 0.785398 : f32

    %0 = qmem.qalloc : i32
    qmem.init %0

    %branch = arith.cmpi eq, %c0, %c1 : i32
    cf.cond_br %branch, ^bb1(%cst_0: f32), ^bb2(%cst_1: f32)

    // CHECK-NEXT: %[[REG_MAIN2:.*]] = qoalahost.call @[[WRAPPER0]]() : () -> i1
  ^bb1(%a1: f32):
    qmem.rot_x %0, %a1
    cf.br ^bb3

  ^bb2(%a2: f32):
    qmem.rot_y %0, %a2
    cf.br ^bb3

  ^bb3:
    %2 = qmem.measure %0 : i1

    // CHECK: qoalahost.return
    qmem.return %2 : i1
  }
}